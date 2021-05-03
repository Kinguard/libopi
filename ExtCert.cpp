#include "ExtCert.h"
#include <libutils/Logger.h>
#include <libutils/FileUtils.h>
#include <libutils/Process.h>

namespace OPI
{

ExtCert::ExtCert() = default;

tuple<int, string> ExtCert::GetExternalCertificates(bool force)
{
    string opts,msg;
	bool res = false;
	int retval = 0;

    if (Utils::File::FileExists(this->certhandler))
    {
	if (force)
	{
	    opts = " -cf";
	}
	else
	{
	    opts = " -c";
	}

	tie(res, std::ignore) = Utils::Process::Exec(this->certhandler+opts);
	if ( res )
	{
	    retval = 1;
	    msg = "Failed to generate external certificate(s)";
	}
	else
	{
	    retval = 0;
	    msg = "External certificates generated.";
	}
    }
    else
    {
	retval = 1;
	msg = "No External Certhandler available";
    }
    return tuple<int,string>(retval,msg);
}

ExtCert::~ExtCert() = default;

} // End NS
