#include "SysInfo.h"

#include <libutils/FileUtils.h>
#include <libutils/String.h>

using namespace Utils;

namespace OPI
{

SysInfo sysinfo;

SysInfo::SysInfo(): numcpus(0), type(TypeUndefined), arch(ArchUndefined)
{
	this->GuessType();
	this->SetupPaths();
}

int SysInfo::NumCpus()
{
	return this->numcpus;
}

SysInfo::SysType SysInfo::Type()
{
	return this->type;
}

SysInfo::SysArch SysInfo::Arch()
{
	return this->arch;
}

string SysInfo::StorageDevicePath()
{
	return this->storagedevicepath+"/"+this->storagedevice+this->storagepartition;
}

string SysInfo::StorageDevice()
{
	return this->storagedevicepath+"/"+this->storagedevice;
}

string SysInfo::StorageDeviceBlock()
{
	return this->storagedevice;
}

string SysInfo::StorageDevicePartition()
{
	return this->storagepartition;
}

string SysInfo::PasswordDevice()
{
	return this->passworddevicepath;
}

SysInfo::~SysInfo()
{

}

bool SysInfo::isOpi()
{
	return sysinfo.Type() == SysInfo::TypeOpi;
}

bool SysInfo::isKeep()
{
	return sysinfo.Type() == SysInfo::TypeKeep;
}

bool SysInfo::isPC()
{
	return sysinfo.Type() == SysInfo::TypePC;
}

void SysInfo::GuessType()
{
	if( ! File::FileExists("/proc/cpuinfo") )
	{
		return;
	}
	list<string> lines = File::GetContent("/proc/cpuinfo");
	for( string line: lines)
	{
		list<string> kval = String::Split(line,":");
		if( kval.size() == 2 )
		{
			string key = String::Trimmed(kval.front(), " \t");
			string val = String::Trimmed(kval.back(), " \t");

			if( key == "processor" )
			{
				this->numcpus++;
			}
			else if( key == "model name")
			{
				if( val.find("Intel") != string::npos )
				{
					this->type = SysInfo::TypePC;
					this->arch = SysInfo::ArchIntel;
				}
				if( val.find("ARMv7") != string::npos )
				{
					this->arch = SysInfo::ArchArm;
				}
			}
			else if( key == "Hardware")
			{
				if( val.find("Generic AM33XX") != string::npos )
				{
					this->type = SysInfo::TypeOpi;
				}
				else if ( val.find("ODROID-XU3") != string::npos )
				{
					this->type = SysInfo::TypeKeep;
				}
			}
		}
	}

	// Have we found a match?
	if( this->type == TypeUndefined )
	{
		// Nopes
		this->type = TypeUnknown;
	}
}

void SysInfo::SetupPaths()
{
	switch( this->type )
	{
	case TypeOpi:
		this->storagedevicepath = "/dev/disk/by-path";
		this->storagedevice = "platform-48060000.mmc";
		this->storagepartition = "-part1";
		this->passworddevicepath = "/dev/sda1";
		break;
	case TypeKeep:
		this->storagedevicepath = "/dev/disk/by-path";
		this->storagedevice = "platform-xhci-hcd.2.auto-usb-0:1.1:1.0-scsi-0:0:0:0";
		this->storagepartition = "-part1";
		this->passworddevicepath = "/dev/disk/by-path/platform-12110000.usb-usb-0:1:1.0-scsi-0:0:0:0-part1";
		break;
	case TypePC:
		this->storagedevicepath = "/dev";
		this->storagedevice = "sdg";
		this->storagepartition = "1";
		this->passworddevicepath = "Undefined";
		break;
	default:
		break;
	}
}

} // Namespace OPI
