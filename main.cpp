/*
** Mutter - Command-line administration tool for Murmur
** 
** Copyright (c) 2010, Jamie Fraser @ MumbleDog
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without 
** modification, are permitted provided that the following conditions are
** met:
**
**    * Redistributions of source code must retain the above copyright 
**      notice, this list of conditions and the following disclaimer.
**    * Redistributions in binary form must reproduce the above copyright
**      notice, this list of conditions and the following disclaimer in the
**      documentation and/or other materials provided with the distribution.
**    * Neither the name MumbleDog, Mumble nor the names of its contributors
**      may be used to endorse or promote products derived from this
**      software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
** IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <iomanip>
#include <Ice/Ice.h>
#include <Murmur.h>

using namespace std;
using namespace Murmur;

#define ACT_CONFPEEK	 1
#define ACT_CONFPOKE	 2
#define ACT_START	 3
#define ACT_STOP	 4
#define ACT_SERVLIST	 5
#define ACT_USERADD	 6
#define ACT_USERDEL	 7
#define ACT_USERPASS	 8
#define ACT_USERLIST	 9
#define ACT_SERVNEW	10
#define ACT_SERVDEL	11

MetaPrx meta;
Ice::Context ctx;
int serverId;

void
config_peek(char *key)
{
	string value;
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	value = server->getConf(key, ctx);
	cout << key << "=" << value << endl;
}

void
config_poke(char *key, char *val)
{
	string value;
	ServerPrx server;

	server = meta->getServer(serverId, ctx);
	server->setConf(key, val, ctx);
	value = server->getConf(key, ctx);
	cout << key << "=" << val << endl;
}

void
config_poke(char *key, string val)
{
	string value;
	ServerPrx server;

	server = meta->getServer(serverId, ctx);
	server->setConf(key, val, ctx);
	value = server->getConf(key, ctx);
	cout << key << "=" << val << endl;
}

void
serv_start(void)
{
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	server->start(ctx);
}

void
serv_stop(void)
{
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	server->stop(ctx);
}

void
serv_new(void)
{
	ServerPrx server;
	int id;

	server = meta->newServer(ctx);
	id = server->id(ctx);
	
	cout << "New server ID: " << id << endl;
}

void
serv_del(void)
{
	ServerPrx server;

	server = meta->getServer(serverId, ctx);
	if (server->isRunning(ctx))
		server->stop(ctx);
	server->_cpp_delete(ctx);
	cout << "Server deleted!" << endl;
}

void
serv_list(void)
{
	int i, id;
	string name, host, port;
	vector<ServerPrx> servers = meta->getAllServers(ctx);

	cout << left << "ID    " << setw(40) << "Server Name" << setw(5) << " On?"
		<< "Host" << endl;
	cout << left << "~~~~~ " << setw(40) << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" 
		<< setw(5) << " ~~~" << "~~~~~~~~~~~~~~~~~~~" << endl;
	
	for (i=0; i < (int)servers.size(); i++)
	{
		id = servers[i]->id(ctx);
		name = servers[i]->getConf("registername", ctx);
		host = servers[i]->getConf("host", ctx);
		port = servers[i]->getConf("port", ctx);
		
		cout << setw(5) << right << id << " " << setw(40) << left << name;
		if (servers[i]->isRunning(ctx))
			cout << " On  ";
		else
			cout << " Off ";
		cout << host << ":" << port << endl;
	}	
}

void
user_delete(char *username)
{
	NameList name;
	IdMap users;
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	name.push_back(username);
	
	users = server->getUserIds(name, ctx);
	if (users[username] < 0)
		throw "Invalid User";
	else 
	{
		server->unregisterUser(users[username], ctx);
		cout << username << " deleted." << endl;
	}
}

void
user_add(char *username)
{
	string name (username);
	ServerPrx server;
	UserInfoMap uinfo;

	uinfo[UserName] = name;
	
	
	server = meta->getServer(serverId, ctx);
	cout << "Enter new password for " << uinfo[UserName] << ": ";
	string pass;
	getline(cin, pass);

	uinfo[UserPassword] = pass;
	server->registerUser(uinfo, ctx);
	cout << "User " << name << " registered!" << endl;
	
}

void
user_pass(char *username)
{
	NameList name;
	IdMap users;
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	name.push_back(username);

	users = server->getUserIds(name, ctx);
	if (users[username] < 0)
		throw "Invalid User";
	else
	{
		UserInfoMap uinfo = server->getRegistration(users[username], ctx);
		cout << "Enter new password for " << uinfo[UserName] << ": ";
		string pass;
		getline(cin, pass);
		
		uinfo[UserPassword] = pass;
		server->updateRegistration(users[username], uinfo, ctx);
		cout << "Password Updated!" << endl;
	}
}

void
user_list(void)
{
	NameMap users;
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	users = server->getRegisteredUsers("", ctx);
	
	for (NameMap::iterator ii=users.begin(); ii != users.end(); ii++)
		cout << setw(8) << right << (*ii).first << " " << left << (*ii).second << endl;
}

void
usage(char *argv[])
{
	cout << "Mutter - (C) 2010 Jamie Fraser @ MumbleDog "
		"- http://www.mumbledog.com/" << endl;
	cout << "Usage: " << argv[0] << " [-i <endpoint>] [-s <server id>]"
		" [-z <secret>] <action>" << endl << endl;
	cout << "Where action is one of:" << endl;
	cout << "\t-C <key> => View configuration key" << endl;
	cout << "\t-C <key> -V <value> => Set configuration key to value" << endl;
	cout << "\t-C <key> -V - => Read configuration value from stdin." << endl;
	cout << "\t-L => List all virtual servers." << endl;
	cout << "\t-S => Start a virtual server." << endl;
	cout << "\t-T => Stop a virtual server." << endl;
	cout << "\t-l => List all users on a virtual server." << endl;
	cout << "\t-a -u <username> => Register a user (reads pass from stdin)" << endl;
	cout << "\t-p -u <username> => Change user's password (reads from stdin)" << endl;
	cout << "\t-d -u <username> => Unregister (delete) a user." << endl;
}

int
main(int argc, char *argv[])
{
	char c;
	char *iceProxy;
	char *iceSecret;
	char *confKey;
	char *confValue;
	char *userName;

	int ret;
	int action;

	Ice::CommunicatorPtr ic;

	/*
	** Sensible defaults
	*/
	iceProxy = (char *)"Meta:tcp -h localhost -p 6502";
	iceSecret = NULL;
	serverId = 1;
	
	confKey = 0;
	confValue = 0;
	userName = 0;
	
	ret = 0;
	action = 0;
	ic = 0;

	/*
	** Parse command line options
	*/
	while ((c = getopt(argc, argv, "adi:lpnrs:u:z:C:LSTV:")) != -1)
		switch (c)
		{
		case 'a':
			action = ACT_USERADD;
			break;
		case 'd':
			action = ACT_USERDEL;
			break;
		case 'i':
			iceProxy = optarg;
			break;
		case 'l':
			action = ACT_USERLIST;
			break;
		case 'p':
			if (!action)
				action = ACT_USERPASS;
			break;
		case 'n':
			action = ACT_SERVNEW;
			break;
		case 'r':
			action = ACT_SERVDEL;
			break;
		case 's':
			serverId = atoi(optarg);
			break;
		case 'u':
			userName = optarg;
			break;
		case 'z':
			iceSecret = optarg;
			break;
		case 'C':
			if (!action)
				action = ACT_CONFPEEK;
			confKey = optarg;
			break;
		case 'L':
			action = ACT_SERVLIST;
			break;
		case 'S':
			action = ACT_START;
			break;
		case 'T':
			action = ACT_STOP;
			break;
		case 'V':
			action = ACT_CONFPOKE;
			confValue = optarg;
			break;
		case '?':
		default:
			usage(argv);
			return -1;
			// OK to return here, ic isn't init yet.
			break; // NOT REACHED
		}

	if (!action) {
		usage(argv);
		ret = -1;
	} else try {
		ic = Ice::initialize(argc, argv);
		Ice::ObjectPrx base = ic->stringToProxy(iceProxy);
		if (iceSecret)
			ctx["secret"] = (string)iceSecret;

		meta = MetaPrx::checkedCast(base);
		if (!meta)
			throw "Ice Error: Invalid Proxy";

		switch (action)
		{
		case ACT_CONFPEEK:
			config_peek(confKey);
			break;
		case ACT_CONFPOKE:
			if (strcmp(confValue, "-") == 0)
			{
				string confVal;
				string line;
				
				while (cin) {
					getline(cin, line);
					confVal = confVal + line + "\n";
				}
				
				config_poke(confKey, confVal);
			}
			else
				config_poke(confKey, confValue);
			break;
		case ACT_START:
			serv_start();
			break;
		case ACT_STOP:
			serv_stop();
			break;
		case ACT_SERVLIST:
			serv_list();
			break;
		case ACT_SERVNEW:
			serv_new();
			break;
		case ACT_SERVDEL:
			serv_del();
			break;
		case ACT_USERADD:
			if (!userName)
				throw "-a requires -u <username>";
			else
				user_add(userName);
			break;
		case ACT_USERDEL:
			if (!userName)
				throw "-d requires -u <username>";
			else
				user_delete(userName);
			break;
		case ACT_USERPASS:
			if (!userName)
				throw "-p requires -u <username>";
			else
				user_pass(userName);
			break;
		case ACT_USERLIST:
			user_list();
			break;
		default:
			// This shouldn't happen...
			usage(argv);
			ret = -1;
		}
	} catch (const Ice::Exception& ex) {
		cerr << ex << endl;
		ret = -1;
	} catch (const char* msg) {
		cerr << msg << endl;
		ret = -1;
	}
	/*
	** Show's over, clean up after Ice
	*/
	if (ic)
		ic->destroy();	

	return ret;
}
