
#include "Secop.h"


#include <libutils/Logger.h>

using namespace Utils;

namespace OPI
{

inline void throw_error(const json& rep)
{
	if( rep.contains("status") && rep["status"].contains("desc") && rep["status"]["desc"].is_string() )
	{
		throw std::runtime_error(rep["status"]["desc"]);
	}
	else
	{
		throw std::runtime_error("Internal error");
	}
}

Secop::Secop(): tid(0), secop("/tmp/secop")
{

}

bool Secop::Init(const string& pwd)
{
	json cmd;

	cmd["cmd"]= "init";
	cmd["pwd"]=pwd;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

Secop::State Secop::Status()
{
	json cmd;

	cmd["cmd"]="status";

	json rep = this->DoCall(cmd);

	if( ! this->CheckReply(rep) )
	{
		throw_error(rep);
	}

	return static_cast<Secop::State>(rep["server"]["state"]);
}

bool Secop::SockAuth()
{
	json cmd;

	cmd["cmd"]= "auth";
	cmd["type"]="socket";

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::PlainAuth(const string& user, const string& pwd)
{
	json cmd;

	cmd["cmd"]		= "auth";
	cmd["type"]		= "plain";
	cmd["username"]	= user;
	cmd["password"]	= pwd;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::CreateUser(const string& user, const string& pwd, const string &display)
{
	json cmd;

	cmd["cmd"]		= "createuser";
	cmd["username"]	= user;
	cmd["password"]	= pwd;
	if( display != "")
	{
		cmd["displayname"] = display;
	}

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::UpdateUserPassword(const string &user, const string &pwd)
{
	json cmd;

	cmd["cmd"]		= "updateuserpassword";
	cmd["username"]	= user;
	cmd["password"]	= pwd;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::RemoveUser(const string& user)
{
	json cmd;

	cmd["cmd"]		= "removeuser";
	cmd["username"]	= user;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}


vector<string> Secop::GetUsers()
{
	json cmd;

	cmd["cmd"]= "getusers";

	json rep = this->DoCall(cmd);

	vector<string> users;
	if( this->CheckReply(rep) )
	{
		for(const auto& x: rep["users"])
		{
			users.push_back( x );
		}
	}
	else
	{
		throw_error(rep);
	}
	return users;
}

vector<string> Secop::GetUserGroups(const string& user)
{
	json cmd;

	cmd["cmd"]		= "getusergroups";
	cmd["username"]	= user;

	json rep = this->DoCall(cmd);

	vector<string> groups;
	if( this->CheckReply(rep) )
	{
		for(const auto& x: rep["groups"])
		{
			groups.push_back( x );
		}
	}
	else
	{
		throw_error(rep);
	}
	return groups;
}

bool Secop::AddAttribute(const string &user, const string &attr, const string &value)
{
	json cmd;

	cmd["cmd"]		= "addattribute";
	cmd["username"]	= user;
	cmd["attribute"]= attr;
	cmd["value"]= value;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::RemoveAttribute(const string &user, const string &attr)
{
	json cmd;

	cmd["cmd"]		= "addattribute";
	cmd["username"]	= user;
	cmd["attribute"]= attr;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

vector<string> Secop::GetAttributes(const string &user)
{
	json cmd;

	cmd["cmd"]		= "getattributes";
	cmd["username"]	= user;

	json rep = this->DoCall(cmd);

	vector<string> attrs;
	if( this->CheckReply(rep) )
	{
		for(const auto& x: rep["attributes"])
		{
			attrs.push_back( x );
		}
	}
	else
	{
		throw_error(rep);
	}
	return attrs;
}

string Secop::GetAttribute(const string &user, const string &attr)
{
	json cmd;

	cmd["cmd"]		= "getattribute";
	cmd["username"]	= user;
	cmd["attribute"] = attr;
	json rep = this->DoCall(cmd);

	if( ! this->CheckReply(rep) )
	{
		throw_error(rep);
	}

	return rep["attribute"];
}
vector<string> Secop::GetServices(const string& user)
{
	json cmd;

	cmd["cmd"]		= "getservices";
	cmd["username"]	= user;

	json rep = this->DoCall(cmd);

	vector<string> services;
	if( this->CheckReply(rep) )
	{
		for(const auto& x: rep["services"])
		{
			services.push_back( x );
		}
	}
	else
	{
		throw_error(rep);
	}
	return services;
}

bool Secop::AddService(const string& user, const string& service)
{
	json cmd;

	cmd["cmd"]			= "addservice";
	cmd["username"]		= user;
	cmd["servicename"]	= service;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::RemoveService(const string& user, const string& service)
{
	json cmd;

	cmd["cmd"]			= "removeservice";
	cmd["username"]		= user;
	cmd["servicename"]	= service;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

vector<string> Secop::GetACL(const string& user, const string& service)
{
	json cmd;

	cmd["cmd"]		= "getacl";
	cmd["username"]	= user;
	cmd["servicename"]	= service;

	json rep = this->DoCall(cmd);

	vector<string> acl;
	if( this->CheckReply(rep) )
	{
		for(const auto& x: rep["acl"])
		{
			acl.push_back( x );
		}
	}
	else
	{
		throw_error(rep);
	}
	return acl;
}

bool Secop::AddACL(const string& user, const string& service, const string& acl)
{
	json cmd;

	cmd["cmd"]			= "addacl";
	cmd["username"]		= user;
	cmd["servicename"]	= service;
	cmd["acl"]			= acl;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::RemoveACL(const string& user, const string& service, const string& acl)
{
	json cmd;

	cmd["cmd"]			= "removeacl";
	cmd["username"]		= user;
	cmd["servicename"]	= service;
	cmd["acl"]			= acl;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::HasACL(const string& user, const string& service, const string& acl)
{
	json cmd;

	cmd["cmd"]			= "hasacl";
	cmd["username"]		= user;
	cmd["servicename"]	= service;
	cmd["acl"]			= acl;

	json rep = this->DoCall(cmd);
	bool ret = false;

	if ( this->CheckReply(rep) )
	{
		ret = rep["hasacl"];
	}
	else
	{
		throw_error(rep);
	}

	return ret;
}

/* Limited, can only add key value string pairs */
bool Secop::AddIdentifier(const string& user, const string& service, const map<string,string>& identifier)
{
	json cmd;

	cmd["cmd"]			= "addidentifier";
	cmd["username"]		= user;
	cmd["servicename"]	= service;

	for(const auto& x: identifier)
	{
		cmd["identifier"][ x.first ] = x.second;
	}

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

/* Identifier has to contain user &| service */
bool Secop::RemoveIdentifier(const string& user, const string& service, const map<string,string>& identifier)
{
	json cmd;

	cmd["cmd"]			= "removeidentifier";
	cmd["username"]		= user;
	cmd["servicename"]	= service;

	for(const auto& x: identifier)
	{
		cmd["identifier"][ x.first ] = x.second;
	}

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

list<map<string,string>> Secop::GetIdentifiers(const string& user, const string& service)
{
	json cmd;

	cmd["cmd"]			= "getidentifiers";
	cmd["username"]		= user;
	cmd["servicename"]	= service;

	json rep = this->DoCall(cmd);

	list<map<string,string> > ret;
	//logg << Logger::Debug << rep.toStyledString()<< lend;
	if ( this->CheckReply(rep) )
	{
		for( auto x: rep["identifiers"] )
		{
			map<string,string> id;
			for( const auto& item: x.items())
			{
				id[ item.key() ] = item.value();
			}
			ret.push_back( id );
		}
	}
	else
	{
		throw_error(rep);
	}

	return ret;
}

bool Secop::AddGroup(const string &group)
{
	json cmd;

	cmd["cmd"]	= "groupadd";
	cmd["group"]= group;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::AddGroupMember(const string &group, const string &member)
{
	json cmd;

	cmd["cmd"]	= "groupaddmember";
	cmd["group"]= group;
	cmd["member"]= member;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);

}

vector<string> Secop::GetGroupMembers(const string &group)
{
	json cmd;

	cmd["cmd"]	= "groupgetmembers";
	cmd["group"]= group;

	json rep = this->DoCall(cmd);

	vector<string> ret;
	if( this->CheckReply(rep) )
	{
		for( const auto& member: rep["members"])
		{
			ret.push_back( member );
		}
	}
	else
	{
		throw_error(rep);
	}

	return ret;
}

vector<string> Secop::GetGroups()
{
	json cmd;

	cmd["cmd"]			= "groupsget";

	json rep = this->DoCall(cmd);

	vector<string> ret;
	if ( this->CheckReply(rep) )
	{
		for(const auto& group: rep["groups"])
		{
			ret.push_back( group );
		}
	}
	else
	{
		throw_error(rep);
	}
	return ret;
}

bool Secop::RemoveGroup(const string &group)
{
	json cmd;

	cmd["cmd"]		= "groupremove";
	cmd["group"]	= group;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::RemoveGroupMember(const string &group, const string &member)
{
	json cmd;

	cmd["cmd"]		= "groupremovemember";
	cmd["group"]	= group;
	cmd["member"]	= member;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::AppAddID(const string &appid)
{
	json cmd;

	cmd["cmd"]		= "createappid";
	cmd["appid"]	= appid;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

vector<string> Secop::AppGetIDs()
{
	json cmd;

	cmd["cmd"]= "getappids";

	json rep = this->DoCall(cmd);

	vector<string> users;
	if( this->CheckReply(rep) )
	{
		for(const auto& x: rep["appids"])
		{
			users.push_back( x );
		}
	}
	else
	{
		throw_error(rep);
	}
	return users;

}

bool Secop::AppRemoveID(const string &appid)
{
	json cmd;

	cmd["cmd"]		= "removeappid";
	cmd["appid"]	= appid;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::AppAddIdentifier(const string &appid, const map<string, string> &identifier)
{
	json cmd;

	cmd["cmd"]			= "addappidentifier";
	cmd["appid"]		= appid;

	for(const auto& x: identifier)
	{
		cmd["identifier"][ x.first ] = x.second;
	}

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

list<map<string, string> > Secop::AppGetIdentifiers(const string &appid)
{
	json cmd;

	cmd["cmd"]			= "getappidentifiers";
	cmd["appid"]		= appid;

	json rep = this->DoCall(cmd);

	list<map<string,string> > ret;
	//logg << Logger::Debug << rep.toStyledString()<< lend;
	if ( this->CheckReply(rep) )
	{
		for( auto x: rep["identifiers"] )
		{
			map<string,string> id;
			for( const auto& item: x.items())
			{
				id[ item.key() ] = item.value();
			}
			ret.push_back( id );
		}
	}
	else
	{
		throw_error(rep);
	}

	return ret;
}

bool Secop::AppRemoveIdentifier(const string &appid, const map<string, string> &identifier)
{
	json cmd;

	cmd["cmd"]			= "appremoveidentifier";
	cmd["appid"]		= appid;

	for(const auto& x: identifier)
	{
		cmd["identifier"][ x.first ] = x.second;
	}

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::AppAddACL(const string &appid, const string &acl)
{
	json cmd;

	cmd["cmd"]			= "addappacl";
	cmd["appid"]		= appid;
	cmd["acl"]			= acl;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

vector<string> Secop::AppGetACL(const string &appid)
{
	json cmd;

	cmd["cmd"]		= "getappacl";
	cmd["appid"]	= appid;

	json rep = this->DoCall(cmd);

	vector<string> acl;
	if( this->CheckReply(rep) )
	{
		for(const auto& x: rep["acl"])
		{
			acl.push_back( x );
		}
	}
	else
	{
		throw_error(rep);
	}
	return acl;

}

bool Secop::AppRemoveACL(const string &appid, const string &acl)
{
	json cmd;

	cmd["cmd"]			= "removeappacl";
	cmd["appid"]		= appid;
	cmd["acl"]			= acl;

	json rep = this->DoCall(cmd);

	return this->CheckReply(rep);
}

bool Secop::AppHasACL(const string &appid, const string &acl)
{
	json cmd;

	cmd["cmd"]			= "hasappacl";
	cmd["appid"]		= appid;
	cmd["acl"]			= acl;

	json rep = this->DoCall(cmd);
	bool ret = false;

	if ( this->CheckReply(rep) )
	{
		ret = rep["hasacl"];
	}
	else
	{
		throw_error(rep);
	}

	return ret;
}

Secop::~Secop() = default;

json Secop::DoCall(json& cmd)
{
	cmd["tid"]=this->tid;
	cmd["version"]=1.0;
	string r = cmd.dump();

	this->secop.Write(r.c_str(), r.size() );

	char buf[16384];
	int rd;

	json resp;

	if( ( rd = this->secop.Read( buf, sizeof(buf) ) ) > 0  )
	{

		try
		{
			resp = json::parse(buf, buf+rd);
		}
		catch (json::parse_error& err)
		{
			logg << Logger::Error << "Failed to parse response: " << err.what()<<lend;
		}
	}

	return resp;
}

bool Secop::CheckReply( const json& val )
{
	bool ret = false;

	if( val.contains("status") && val["status"].is_object() )
	{
		ret = val["status"]["value"].get<int>() == 0;
	}

	return ret;
}

} // End NS
