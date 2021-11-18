#include <iostream>
#include <stdexcept>
#include <sstream>
#include <list>

#include <libutils/String.h>
#include <libutils/FileUtils.h>
#include <libutils/ConfigFile.h>
#include <libutils/Process.h>
#include <libutils/HttpStatusCodes.h>

#include <resolv.h>

#include "DnsHelper.h"
#include "AuthServer.h"
#include "SmtpConfig.h"
#include "SysConfig.h"
#include "Config.h"

// Forwards for helpers

static inline string getmatchstring(const string& line, Regex::Match m);
static string gethost(const string & host);
static string getport(const string &host);

// Convenience defines
#define SCFG	(OPI::SysConfig())
#define SAREA (SCFG.GetKeyAsString("filesystem","storagemount"))
#define MCFG(opt)  (SAREA+SCFG.GetKeyAsString("mail", opt))
#define CFGOPT(scope, key) (SCFG.GetKeyAsString(scope, key))
#define IS_OP	(SCFG.HasKey("dns", "provider") && SCFG.GetKeyAsString("dns", "provider") == "OpenProducts")

using namespace Utils::HTTP;

// MCFG accesses a config option in mail scope prefixed with storageareapath

namespace OPI
{

SmtpClientConfig::SmtpClientConfig(const string &path, PostConfInterfacePtr pi):
	postconf(pi),
	path(path),
	lreg("^(\\S+)\\s+(.*):(.*)")
{
	this->ReadConfig();
}

void SmtpClientConfig::ReadConfig()
{
	this->postconf->ReadConfig();
	this->_parsesasl();

	// Bugfix for #204 "Sätta relay host funkar inte" when relayhost left empty but sasl enabled
	if ( this->postconf->getRelayhost() == "" )
	{
		this->postconf->setEnable(false);
		return;
	}

	if( ! this->postconf->getEnable() )
	{
		return;
	}

	if( this->passwd.find(this->postconf->getRelayhost() ) != this->passwd.end() )
	{
		this->user = this->passwd[this->postconf->getRelayhost() ].user;
		this->password = this->passwd[this->postconf->getRelayhost()].pass;
	}
	else
	{
		throw runtime_error(string("Failed to find user credentials for ")+this->postconf->getRelayhost() );
	}

}

void SmtpClientConfig::WriteConfig()
{
	this->postconf->WriteConfig();
	this->_writesasal();
}

passwdline SmtpClientConfig::GetConfig()
{
	passwdline cfg;

	cfg.host = gethost( this->postconf->getRelayhost() );
	cfg.port = getport( this->postconf->getRelayhost() );
	cfg.enabled = this->postconf->getEnable();
	cfg.user = this->user;
	cfg.pass = this->password;

	return cfg;
}

void SmtpClientConfig::SetConfig(const passwdline &cfg)
{

	stringstream host;
	// Make sure we have no reminder config
	this->passwd.clear();

	if( cfg.host != "")
	{
		host << "["<<cfg.host<<"]";
		if( cfg.port != "" )
		{
			host << ":" << cfg.port;
		}

		this->passwd[host.str()]=cfg;
	}

	this->postconf->setEnable( cfg.enabled );
	this->postconf->setRelayhost( host.str() );
}

void SmtpClientConfig::dump()
{
	cout << "Sasl config\n";
	cout << "-----------\n";
	cout << "Enabled   : "<< this->postconf->getEnable() <<endl;
	cout << "Relayhost : "<< this->postconf->getRelayhost() <<endl;
	cout << "-----------\n";
	cout << "Username  : "<< this->user<<endl;
	cout << "Password  : "<< this->password<<endl;
#if 0
	for( const auto& p : this->passwd )
	{
		cout << "-----------\n";
		const passwdline line = p.second;
		cout << "Host : "<< line.host << endl;
		cout << "Port : "<< line.port << endl;
		cout << "User : "<< line.user << endl;
		cout << "Pass : "<< line.pass << endl;
	}
#endif
}

SmtpClientConfig::~SmtpClientConfig() = default;

void SmtpClientConfig::_parsesasl()
{
	list<string> plines = File::GetContent( this->path );
	for( const auto& line: plines )
	{
		vector<Regex::Match> m = this->lreg.DoMatch(line);
		if( m.size() == 4 )
		{
			string host = getmatchstring( line, m[1]);

			passwdline pwline(
			{
				true, /* Hack since we reuse this for getters/setters */
				gethost( getmatchstring( line, m[1]) ),
				getport( getmatchstring( line, m[1]) ),
				getmatchstring( line, m[2]),
				getmatchstring( line, m[3])
			});

			this->passwd[host] = pwline;
			//this->passwd[host] = "["+user + "] [" + pass+"] ["+port+"]";
		}
	}

}

void SmtpClientConfig::_writesasal()
{
	stringstream out;

	for( auto& line: this->passwd )
	{
		out << line.first <<"\t"<<line.second.user<<":"<<line.second.pass<<endl;
	}
	//cout << endl << "--------------"<<endl << out.str()<<endl;
	File::Write(this->path, out.str(), File::UserRW );

	Process::Exec( "/usr/sbin/postmap "+ this->path );
}

/*
 *
 * PostConf interface implementation.
 *
 */

string PostConfInterface::getRelayhost() const
{
	return relayhost;
}

void PostConfInterface::setRelayhost(const string &value)
{
	relayhost = value;
}
bool PostConfInterface::getEnable() const
{
	return enable;
}

void PostConfInterface::setEnable(bool value)
{
	enable = value;
}

/*
 *
 * Postconf implementation
 *
 */

void Postconf::ReadConfig()
{
	const char* cmd = "/usr/sbin/postconf  relayhost smtp_sasl_auth_enable";
	string ret;

	tie( ignore, ret) = Process::Exec( cmd );

	list<string> lines = String::Split(ret,"\n");

	if( lines.size() != 2 )
	{
		throw runtime_error("Unable to parse postconf output");
	}

	list<string> words = String::Split( lines.front(),"=");
	if( words.size() == 2 )
	{
		this->relayhost = String::Trimmed( words.back()," \t");
	}

	this->enable = String::Trimmed( String::Split( lines.back(),"=").back()," \t") == "yes";

#if 0
	for( const auto& line: lines)
	{
		cout << "Line {"<<line<<"}"<<endl;
	}
#endif
}

void Postconf::WriteConfig()
{
	stringstream cmd;

	cmd << "/usr/sbin/postconf -e "
		<<"relayhost=\""<<this->relayhost<<"\" "
	   << "smtp_sasl_auth_enable=\"";
	if ( this->enable )
	{
		cmd << "yes";
	}
	else
	{
		cmd << "no";
	}

	cmd <<"\"";

	Process::Exec( cmd.str() );

	// cout << cmd.str()<<endl;
}


SmtpConfig::SmtpConfig(const string &path): cfg(path)
{
	this->is_op = IS_OP;
	this->getConfig();
}

SmtpConfig::SmtpMode SmtpConfig::GetMode()
{
	return this->mode;
}

void SmtpConfig::SetStandAloneMode()
{

	if( this->mode == SmtpMode::OPI )
	{
		// No change
		return;
	}

	// Else custom or relay, reset
	passwdline pw;
	pw.enabled = false;

	SmtpClientConfig scli( MCFG("saslpasswd") );
	scli.SetConfig( pw );

	scli.WriteConfig();

	if( this->is_op && this->mode == SmtpMode::OPRelay && this->opconf.receive )
	{
		// We used OP relay and OP configed to handle our mail, change
		this->setMX( false );
	}

	this->opconf = { false, false};
	this->customconf = {"","","",""};

	this->mode = SmtpMode::OPI;
}

void SmtpConfig::SetOPRelayMode(OPRelayConf &conf)
{
	if( ! this->is_op )
	{
		throw std::runtime_error("Trying to set relay mode on none OP system");
	}

	if( !conf.send && ! conf.receive )
	{
		throw runtime_error("Missing argument for relay settings");
	}

	// Should we use OP server as outgoing relay server?
	if( conf.send )
	{

		stringstream cmd;

		cmd << "/usr/sbin/postconf -e "
			   << "smtp_tls_cert_file=/etc/opi/opi.cert "
			   << "smtp_tls_key_file=/etc/opi/dnspriv.pem";
		Process::Exec( cmd.str() );

		passwdline pw;
		pw.enabled = false;
		pw.host = "op-mail.openproducts.com";
		pw.port="587";

		SmtpClientConfig scli( MCFG("saslpasswd") );
		scli.SetConfig( pw );

		scli.WriteConfig();
	}
	else
	{
		// Make sure we have no leftover config
		passwdline pw;
		pw.enabled = false;

		SmtpClientConfig scli( MCFG("saslpasswd") );
		scli.SetConfig( pw );

		scli.WriteConfig();
	}

	// Tell OP server to forward mail to us!
	if( conf.receive )
	{
		this->setMX( true );
	}
	else if( this->mode == SmtpMode::OPRelay && this->opconf.receive )
	{
		// We used relaymode earlier disable
		this->setMX( false );
	}

	this->opconf = conf;
	this->mode = SmtpMode::OPRelay;
}

OPRelayConf SmtpConfig::GetOPRelayConfig()
{
	if( this->mode != SmtpMode::OPRelay )
	{
		throw runtime_error("Not in OP relay mode");
	}
	return this->opconf;
}

void SmtpConfig::SetCustomMode(OPCustomConf &conf)
{
	passwdline cf;

	cf.host = conf.host;
	cf.port = conf.port;
	cf.enabled = (conf.user != "") && ( conf.pass != "" );

	if( cf.enabled )
	{
		cf.user = conf.user;
		cf.pass = conf.pass;
	}
	this->cfg.SetConfig( cf );
	this->cfg.WriteConfig();

	if( this->is_op && this->mode == SmtpMode::OPRelay && this->opconf.receive )
	{
		this->setMX( false );
	}

	this->customconf = conf;
	this->mode = SmtpMode::Custom;
}

OPCustomConf SmtpConfig::GetOPCustomConfig()
{
	if( this->mode != SmtpMode::Custom )
	{
		throw runtime_error("Not in custom mode");
	}
	return this->customconf;
}

SmtpConfig::~SmtpConfig() = default;

void SmtpConfig::getConfig()
{
	SysConfig sysconf;
	passwdline pass = cfg.GetConfig();

	string name = sysconf.GetKeyAsString("hostinfo", "hostname");
	string domain = sysconf.GetKeyAsString("hostinfo", "domain");

	this->opiname = name+"."+domain;
	if( this->is_op )
	{
		this->unit_id = sysconf.GetKeyAsString("hostinfo", "unitid");
	}
    const string relay = sysconf.GetKeyAsString("mail", "oprelayserver");

	// OP relay?
	if( this->is_op && this->checkMX( this->opiname ) )
	{
		this->mode = SmtpMode::OPRelay;
		this->opconf.receive = true;

		if( pass.host == relay )
		{
			this->opconf.send = true;
		}
		else
		{
			this->opconf.send = false;
		}

		return;
	}

	// OP relay send only?
	if( this->is_op &&  pass.host == relay )
	{
		this->mode = SmtpMode::OPRelay;
		this->opconf.receive = false;
		this->opconf.send = true;
		return;
	}

	// OPI stand alone?
	if( pass.host == "" )
	{
		this->mode = SmtpMode::OPI;
		return;
	}

	// Must be custom mode
	this->mode = SmtpMode::Custom;
	this->customconf.host = pass.host;
	this->customconf.pass = pass.pass;
	this->customconf.port = pass.port;
	this->customconf.user = pass.user;
}

bool SmtpConfig::checkMX(const string& name)
{

	if( ! this->is_op )
	{
		throw std::runtime_error("Trying to get MX on none OP system");
	}

	OPI::AuthServer auth(this->unit_id);

	int resultcode;
	tie(resultcode, std::ignore) = auth.CheckMXPointer(name);

	return resultcode == Status::Ok;
}

void SmtpConfig::setMX(bool mxmode)
{
	if( ! this->is_op )
	{
		throw std::runtime_error("Trying to set MX on none OP system");
	}

	int resultcode;
	json ret;

	OPI::AuthServer s( this->unit_id );

	tie(resultcode, ret) = s.Login();

	if( resultcode != Status::Ok )
	{
		throw runtime_error("Unable to authenticate with backend server");
	}
	string token = ret["token"];

	tie(resultcode, ret) = s.UpdateMXPointer(mxmode, token);
	if( resultcode != Status::Ok )
	{
		throw runtime_error("Unable to update MX settings");
	}
}

} // End namespace

static inline string getmatchstring(const string& line, Regex::Match m)
{
	return line.substr(m.rm_so, m.rm_eo - m.rm_so);
}


static string gethost(const string & host)
{
	list<string> parts = String::Split(host,":");
	if( parts.size() == 2  )
	{
		return String::Trimmed( parts.front(),"[]");
	}
	if( parts.size() == 1 )
	{
		return String::Trimmed( host,"[]");
	}
	return "";
}

static string getport(const string &host)
{
	list<string> parts = String::Split(host,":");
	if( parts.size() == 2  )
	{
		return parts.back();
	}
	return "";
}
