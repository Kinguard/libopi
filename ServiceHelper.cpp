
#include <cstdlib>
#include <string>

#include <libutils/FileUtils.h>
#include <libutils/Process.h>
#include <libutils/String.h>

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
	return sysop("is-active", service);
}

bool IsEnabled(const string &service)
{
	return sysop("is-enabled", service);
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

} // End NS
} // End NS

