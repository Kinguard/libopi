
#include <cstdlib>
#include <string>
#include <map>

#include <libutils/FileUtils.h>
#include <libutils/Process.h>
#include <libutils/String.h>

/*
 *
 * TODO: Refactor and use d-bus api?
 *
 */


using namespace std;
using namespace Utils;

namespace OPI {
namespace ServiceHelper {

static bool sysop(const string& operation, const string& service)
{
	bool result = false;

	tie(result, std::ignore) = Process::Exec("/bin/systemctl -q " + operation +" "+ service + " &> /dev/null");

	return result;
}

bool Start(const string& service)
{
	return sysop("start", service);
}

bool Stop(const string& service)
{
	return sysop("stop", service);
}

bool Enable(const string& service)
{
	return sysop("enable", service);
}

bool Disable(const string& service)
{
	return sysop("disable", service);
}

bool Reload(const string &service)
{
	return sysop("reload", service);
}

bool Restart(const string &service)
{
	return sysop("restart", service);
}


bool IsRunning(const string &service)
{
	bool result = false;
	string rets;

	tie(result, rets) = Process::Exec("/bin/systemctl is-active "+ service + " 2> /dev/null");

	rets = Utils::String::Chomp(rets);

	if( result && rets == "active")
	{
		return true;
	}

	return false;
}

bool IsEnabled(const string &service)
{
	bool result = false;
	string rets;

	tie(result, rets) = Process::Exec("/bin/systemctl is-enabled "+ service + " 2> /dev/null");

	rets = Utils::String::Chomp(rets);

	if( result && rets == "enabled")
	{
		return true;
	}

	return false;
}

/* TODO: is this used? Concider refactor to use systemctl show and parse output or remove */
pid_t GetPid(const string& service)
{
	string pidfile;
	pid_t pid=0;

	pidfile="/run/"+service+".pid";

	if( ! File::FileExists( pidfile ) )
	{
		pidfile="/var/run/"+service+".pid";
	}

	if( File::FileExists( pidfile ) )
	{
		string s_pid=String::Chomp(File::GetContentAsString(pidfile));

		if(File::DirExists("/proc/"+s_pid))
		{
			pid = std::stoi(s_pid);
		}
	}

	return pid;
}

bool IsAvailable(const string &service)
{
	static const map<string, string> servicemap =
	{
		{"ssh", "openssh-server"},
		{"dovecot", "dovecot-imapd"}
	};

	bool result = false;
	string pkg;

	if( servicemap.find(service) != servicemap.end() )
	{
		pkg = servicemap.at(service);
	}
	else
	{
		pkg = service;
	}

	tie(result, std::ignore) = Process::Exec("/usr/bin/dpkg-query -s " + pkg + " > /dev/null 2>&1 ");

	return result;
}

} // End NS
} // End NS

