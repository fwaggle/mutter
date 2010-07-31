/*
** mutter - command line administration tool for Murmur
** (C) 2009-2010 MumbleDog
** http://www.mumbledog.com/
**
** Author: Jamie Fraser <jamie.f@sabrienix.com>
** Released under the terms of a new-style BSD license.
*/

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <Ice/Ice.h>
#include <Murmur.h>

using namespace std;
using namespace Murmur;

#define ACT_LIST 	1
#define ACT_START 	2
#define ACT_STOP	3
#define ACT_PASS	4
#define ACT_AUSER	5
#define ACT_DUSER	6

int
main(int argc, char *argv[])
{
	int ret = 0;
	int port = 6502;
	int c;
	int sid = 1;
	char *host = (char *)"127.0.0.1";
	char *IceProxy;
	Ice::CommunicatorPtr ic;
	char *ckey = 0;
	char *cval = 0;
	char *user = 0;
	char *secret = 0;
	int action = 0;

	while ((c = getopt(argc, argv, "h:p:s:z:A:C:D:PU:V:LST")) != -1)
		switch(c)
		{
		case 'h':
			host = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 's':
			sid = atoi(optarg);
			break;
		case 'z':
			secret = optarg;
			break;
		case 'C':
			ckey = optarg;
			break;
		case 'V':
			cval = optarg;
			break;
		case 'L':
			action = ACT_LIST;
			break;
		case 'S':
			action = ACT_START;
			break;
		case 'T':
			action = ACT_STOP;
			break;
		case 'A':
			user = optarg;
			action = ACT_AUSER;
			break;
		case 'D':
			user = optarg;
			action = ACT_DUSER;
			break;
		case 'U':
			user = optarg;
			break;
		case 'P':
			action = ACT_PASS;
			break;
		}
	
	ret = asprintf(&IceProxy, "Meta:tcp -h %s -p %d", host, port);
	if (!ret)
		return -1;

	try {
		ic = Ice::initialize(argc, argv);
		Ice::ObjectPrx base = ic->stringToProxy(IceProxy);
		Ice::Context ctx;
		if (secret) {
			string sec (secret);
			ctx["secret"] = sec;
		}
		MetaPrx meta = MetaPrx::checkedCast(base);
		if (!meta)
			throw "Invalid proxy";

		ServerPrx server = meta->getServer(sid, ctx);
		vector<ServerPrx> servers = meta->getAllServers(ctx);

		// ugly hack, i'll fix this later when i wrap everything
		// in functions. :D
		char *usr = user;
		if (!usr)
			usr = (char *)"";
			
		string uname (usr);
		NameList n;
		IdMap users;

		if (action) {
			switch (action)
			{
			case ACT_LIST:
				int i;
				for (i=0; i < (int)servers.size(); i++)
				{
					string value = servers[i]->getConf("registername", ctx);
					if (servers[i]->isRunning(ctx))
					cout << setw(5) << right << i << "\t" << setw(50) << left 
						<< value << "\tOnline" << endl;
					else
						cout << setw(5) << right << i << "\t" << setw(50) << left 
						<< value << "\tOffline" << endl;
				}
				break;
			case ACT_START:
				server->start(ctx);
				break;
			case ACT_STOP:
				server->stop(ctx);
				break;
			case ACT_DUSER:
				n.push_back(uname);

				users = server->getUserIds(n, ctx);
				if (users[user] < 0) {
					throw "User not found";
				} else {
					server->unregisterUser(users[user], ctx);
					cout << user << " deleted." << endl;
				}
				break;
			case ACT_PASS:
				if (!user)
					throw "-P requires -U";
				else {
/*					std::string uname (user);
					NameList n;
*/					n.push_back(uname);

					users = server->getUserIds(n, ctx);
					if (users[user] < 0) {
						throw "User not found";
					} else {
						UserInfoMap uinfo = server->getRegistration(users[user], ctx);
						cout << "Changing password for: " << uinfo[UserName] << "... " << endl;
						string pass;
						getline(cin, pass);
						
						uinfo[UserPassword] = pass;
						server->updateRegistration(users[user], uinfo, ctx);
						cout << "done!" << endl;
					}
				}
				break;
			case ACT_AUSER:
				UserInfoMap uinfo;
				std::string uname (user);

				uinfo[UserName] = uname;

				cout << "Setting password for: " << uinfo[UserName] << "... " << endl;
				string pass;
				getline(cin, pass);
						
				uinfo[UserPassword] = pass;
				server->registerUser(uinfo, ctx);
				cout << "done!" << endl;
				break;
			}
		}
		else if (cval && ckey)
		{
			// We're setting a value
			server->setConf(ckey, cval);
			string value = server->getConf(ckey, ctx);
			cout << ckey << "=" << value << endl;
		}
		else if (ckey)
		{
			// We're just checking a value
			string value = server->getConf(ckey, ctx);
			cout << ckey << "=" << value << endl;
		}
	} catch (const Ice::Exception& ex) {
		cerr << ex << endl;
		ret = -1;
	} catch (const char* msg) {
		cerr << msg << endl;
		ret = -1;
	}
	
	if (ic)
		ic->destroy();
	return ret;
}
