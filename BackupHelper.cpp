#include "BackupHelper.h"

#include <sstream>
#include <tuple>

#include <ext/stdio_filebuf.h>
#include <libutils/Exceptions.h>
#include <libutils/FileUtils.h>
#include <libutils/Process.h>
#include <unistd.h>

#define MOUNTCMD	"/usr/share/opi-backup/mount_fs.sh"
#define UMOUNTCMD	"/usr/share/opi-backup/umount_fs.sh"
#define LOCALMOUNT	"/tmp/localbackup"
#define REMOTEMOUNT	"/tmp/remotebackup"

namespace OPI
{

BackupHelper::BackupHelper(const string& pwd, BackupInterfacePtr iface): pwd(pwd), iface(iface)
{

}

bool BackupHelper::MountLocal(const string &configpath)
{
	return this->iface->MountLocal( configpath );
}

bool BackupHelper::MountRemote(const string &configpath)
{
	return this->iface->MountRemote( configpath );
}

list<string> BackupHelper::GetLocalBackups()
{
	return this->iface->GetLocalBackups();
}

list<string> BackupHelper::GetRemoteBackups()
{
	return this->iface->GetRemoteBackups();
}

BackupHelper::~BackupHelper()
{
	unlink( this->tmpfilename );
}

void BackupHelper::CreateConfig()
{
	sprintf( this->tmpfilename,"/tmp/bkcfgXXXXXX");

	int fd = mkstemp(this->tmpfilename);

	if( fd <0 )
	{
		throw Utils::ErrnoException("Failed to create file");
	}

	__gnu_cxx::stdio_filebuf<char> fb( fd, std::ios::out);

	ostream out(&fb);

	out <<"[s3op]\n"
		<<"storage-url: s3op://\n"
		<<"backend-login: NotUsed\n"
		<<"backend-password: NotUsed\n"
		<<"fs-passphrase: "<< this->pwd<<"\n"
		<<"\n"
		<< "[local]\n"
		<< "storage-url: local://\n"
		<< "fs-passphrase: "<< this->pwd << "\n";

	out << flush;

}

string BackupHelper::GetConfigFile()
{
	return this->tmpfilename;
}

OPIBackup::OPIBackup()
{

}

static bool trymount(const string& path, bool local)
{
	bool result;

	stringstream ss;

	ss << MOUNTCMD << " -r -a " << path << "-b ";
	if( local )
	{
		ss << "\"local://\" -m " << LOCALMOUNT;
	}
	else
	{
		ss << "\"s3op://\" -m " << REMOTEMOUNT;
	}

	tie(result, std::ignore ) = Utils::Process::Exec( ss.str() );

	return result;
}

bool OPIBackup::MountLocal(const string &configpath)
{
	if( ! Utils::File::DirExists( LOCALMOUNT ) )
	{
		Utils::File::MkDir( LOCALMOUNT, 0700);
	}

	return trymount( configpath, true);
}

bool OPIBackup::MountRemote(const string &configpath)
{
	if( ! Utils::File::DirExists( REMOTEMOUNT ) )
	{
		Utils::File::MkDir( REMOTEMOUNT, 0700);
	}

	return trymount( configpath, false);
}

static list<string> getbackups(bool local)
{
	string bp;
	if( local )
	{
		bp = LOCALMOUNT "/*";
	}
	else
	{
		bp = REMOTEMOUNT "/*";
	}

	list<string> ret, res = Utils::File::Glob( bp );
	for( const auto& r: res)
	{

		if( Utils::File::GetFileName( r ) != "lost+found" )
		{
			ret.push_back( r );
		}
	}
	return ret;
}

list<string> OPIBackup::GetLocalBackups()
{
	return getbackups( true );
}

list<string> OPIBackup::GetRemoteBackups()
{
	return getbackups(false);
}


static bool doumount(bool local)
{
	bool result;

	stringstream ss;

	ss << UMOUNTCMD;
	if( local )
	{
		ss << " -m " << LOCALMOUNT;
	}
	else
	{
		ss << " -m " << REMOTEMOUNT;
	}

	tie(result, std::ignore ) = Utils::Process::Exec( ss.str() );

	return result;
}

void OPIBackup::UmountLocal()
{
	doumount( true );
}

void OPIBackup::UmountRemote()
{
	doumount( false );
}

OPIBackup::~OPIBackup()
{

}

}
