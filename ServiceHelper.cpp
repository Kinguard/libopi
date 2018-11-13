
#include <stdlib.h>
#include <string>

#include <stdlib.h>

#include <libutils/FileUtils.h>
#include <libutils/Process.h>
#include <libutils/String.h>

using namespace std;
using namespace Utils;

namespace OPI {
namespace ServiceHelper {

bool Start(const string& service)
{
	bool result;

	tie(result, std::ignore) = Process::Exec("/bin/systemctl -q start " + service + " &> /dev/null");

	return result;
}

bool Stop(const string& service)
{
	bool result;

	tie(result, std::ignore) = Process::Exec("/bin/systemctl -q stop " + service + " &> /dev/null");

	return result;
}

bool Reload(const string &service)
{
	bool result;

	tie(result, std::ignore) = Process::Exec("/bin/systemctl -q reload " + service + " &> /dev/null");

	return result;
}

/* TODO: is this used? Concider refactor to use systemctl show and parse output or remove */

pid_t GetPid(const string& service)
{
	string pidfile;
	pid_t pid=0;

	pidfile="/var/run/"+service+".pid";

	if( File::FileExists( pidfile ) )
	{
		string s_pid=String::Chomp(File::GetContentAsString(pidfile));

		if(File::DirExists("/proc/"+s_pid))
		{
			pid=atoi(s_pid.c_str());
		}
	}

	return pid;
}


bool IsRunning(const string &service)
{
	bool result;

	tie(result, std::ignore) = Process::Exec("/bin/systemctl -q is-active " + service + " &> /dev/null");

	return result;
}

} // End NS
} // End NS

