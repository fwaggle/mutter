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
	int action = 0;

	while ((c = getopt(argc, argv, "h:p:s:C:PU:V:LST")) != -1)
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
		MetaPrx meta = MetaPrx::checkedCast(base);
		if (!meta)
			throw "Invalid proxy";

		ServerPrx server = meta->getServer(sid);
		vector<ServerPrx> servers = meta->getAllServers();

		if (action) {
			switch (action)
			{
			case ACT_LIST:
				int i;
				for (i=0; i < (int)servers.size(); i++)
				{
					string value = servers[i]->getConf("registername");
					if (servers[i]->isRunning())
					cout << setw(5) << right << i << "\t" << setw(50) << left 
						<< value << "\tOnline" << endl;
					else
						cout << setw(5) << right << i << "\t" << setw(50) << left 
						<< value << "\tOffline" << endl;
				}
				break;
			case ACT_START:
				server->start();
				break;
			case ACT_STOP:
				server->stop();
				break;
			case ACT_PASS:
				if (!user)
					throw "-P requires -U";
				else {
					std::string uname (user);
					NameList n;
					n.push_back(uname);

					IdMap users = server->getUserIds(n);
					if (users[user] < 0) {
						throw "User not found";
					} else {
						UserInfoMap uinfo = server->getRegistration(users[user]);
						cout << "Changing password for: " << uinfo[UserName] << "... ";
						string pass;
						getline(cin, pass);
						
						uinfo[UserPassword] = pass;
						server->updateRegistration(users[user], uinfo);
						cout << "done!" << endl;
					}
				}
				break;
			}
		}
		else if (cval && ckey)
		{
			// We're setting a value
			server->setConf(ckey, cval);
			string value = server->getConf(ckey);
			cout << ckey << "=" << value << endl;
		}
		else if (ckey)
		{
			// We're just checking a value
			string value = server->getConf(ckey);
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
