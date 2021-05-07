/*
*	This file is part of libopi.
*
*	Copyright (c) 2016-2018 Tor Krill <tor@openproducts.com>
*                           PA Nilsson <pa@openproducts.com>
*
*	libopi is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Affero General Public License as published
*	by the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	libopi is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU Affero General Public License for more details.
*
*	You should have received a copy of the GNU Affero General Public
*	License along with libopi.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SysInfo.h"
#include "NetworkConfig.h"
#include "Config.h"

#include <map>
#include <iostream>
#include <algorithm>

#include <json/json.h>

#include <libutils/FileUtils.h>
#include <libutils/String.h>

static constexpr const char* OS_INFOFILE="/etc/os-release";
static constexpr const char* CPU_INFOFILE="/proc/cpuinfo";

using namespace Utils;

namespace OPI
{

SysInfo sysinfo;

SysInfo::SysInfo()
{
	this->GuessType();
	this->SetupPaths();
	this->GetOSInfo();
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

SysInfo::OSType SysInfo::OS()
{
	return this->os;
}

string SysInfo::OSVersion()
{
	return this->osversion;
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

string SysInfo::NetworkDevice()
{
	return this->networkdevice;
}

string SysInfo::SerialNumberDevice()
{
    return this->serialnbrdevice;
}

string SysInfo::SerialNumber()
{

    // serial number is always places as the last parameter in the flash, so it is in the last element
    // Serial is in the format of 2712KEEP1234 (12 chars)
    char data[250];
    const char* p = data;
    vector<string> v_serial;
    string serial = "Undefined";
    list<string> allowed_patterns;
    size_t found;
	int offset = 0, serial_size = 12;

    if (! File::FileExists(this->serialnbrdevice) )
    {
        return "Undefined";
    }
    File::Read(this->serialnbrdevice, data, 200);

    while ( string(p).length() > 0 )
    {
        v_serial.emplace_back(p);
        p += v_serial.back().size() + 1;
	}

    switch ( this->type )
    {
    case TypeOpi:
        // OPI does not have the "serial=" identifier
        allowed_patterns.emplace_back("OP-I");
        allowed_patterns.emplace_back("BBBK");  // used in the first batch of OPI's
        offset = -4;
        break;
    case TypeArmada:
    case TypePC:
        allowed_patterns.emplace_back("serial=");
        offset = 7;
        break;

    case TypeXu4:
    case TypeOlimexA20:
    default:
        break;
    }
    for (auto const& pattern : allowed_patterns) {
        if (pattern.size()) {
            found = v_serial.back().find(pattern);
            if (found != std::string::npos)
            {
                serial= v_serial.back().substr(found+offset,serial_size);
            }
        }
    }
    return serial;
}

string SysInfo::BackupRootPath()
{
    return this->backuprootpath;
}

bool SysInfo::isArmada()
{
	return sysinfo.Type() == SysInfo::TypeArmada;
}

bool SysInfo::isOpi()
{
	return sysinfo.Type() == SysInfo::TypeOpi;
}

bool SysInfo::isXu4()
{
	return sysinfo.Type() == SysInfo::TypeXu4;
}

bool SysInfo::isOlimexA20()
{
	return sysinfo.Type() == SysInfo::TypeOlimexA20;
}

bool SysInfo::isPC()
{
	return sysinfo.Type() == SysInfo::TypePC;
}

bool SysInfo::isRPI3()
{
	return sysinfo.Type() == SysInfo::TypeRPI3;
}

bool SysInfo::isRPI4()
{
	return sysinfo.Type() == SysInfo::TypeRPI4;
}

bool SysInfo::useLVM()
{
	SysType t = sysinfo.Type();
	if( t == SysInfo::TypeOpi || t == SysInfo::TypeOlimexA20 )
	{
		return false;
	}

	return true;
}

bool SysInfo::useLUKS()
{
	// All platforms use LUKS atm
	return true;
}

bool SysInfo::fixedStorage()
{
	SysType t = sysinfo.Type();
	// OPI and Keep has "fixed" storage and we should use that
	if( t == SysInfo::TypeOpi || t == SysInfo::TypeArmada )
	{
		return true;
	}

	return false;
}

SysInfo::SysType SysInfo::TypeFromName(const string &devname)
{
	static const map<string, SysInfo::SysType> devtypemap =
	{
		{"opi"		,SysInfo::TypeOpi },
		{"xu4"		,SysInfo::TypeXu4 },
		{"olimexa20",SysInfo::TypeOlimexA20 },
		{"armada"	,SysInfo::TypeArmada },
		{"pc"		,SysInfo::TypePC },
		{"rpi3"		,SysInfo::TypeRPI3},
		{"rpi4"		,SysInfo::TypeRPI4}
	};
    string dev = devname;
    transform(dev.begin(), dev.end(), dev.begin(), ::tolower);

    if( devtypemap.find(dev) != devtypemap.end() )
	{
		return devtypemap.at(dev);
	}
	return SysType::TypeUnknown;
}

void SysInfo::GetOSInfo()
{
	static const map<string, SysInfo::OSType> ostypemap =
	{
		{"debian", OSType::OSDebian},
		{"raspbian", OSType::OSRaspbian},
		{"ubuntu", OSType::OSUbuntu}
	};

	if( !File::FileExists(OS_INFOFILE) && ! File::LinkExists(OS_INFOFILE) )
	{
		return;
	}

	list<string> lines = File::GetContent(OS_INFOFILE);
	for( const string& line: lines)
	{
		list<string> kval = String::Split(line,"=");
		if( kval.size() != 2 )
		{
			continue;
		}

		this->osinfo[kval.front()] = String::Trimmed(kval.back(), "\"\t ");
	}

	if( this->osinfo.find("ID") != this->osinfo.end() )
	{
		if( ostypemap.find(this->osinfo["ID"]) != ostypemap.end() )
		{
			this->os = ostypemap.at(this->osinfo["ID"] );
		}
	}

	if( this->osinfo.find("VERSION_CODENAME") != this->osinfo.end() )
	{
		this->osversion = this->osinfo["VERSION_CODENAME"];
	}
}

void SysInfo::GuessType()
{
	if( ! File::FileExists(CPU_INFOFILE) )
	{
		return;
	}
	list<string> lines = File::GetContent(CPU_INFOFILE);
	for( const string& line: lines)
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
					this->type = SysInfo::TypeXu4;
				}
				else if ( val.find("sun7i") != string::npos )
				{
					this->type = SysInfo::TypeOlimexA20;
				}
				else if ( val.find("Marvell Armada") != string::npos )
				{
					this->type = SysInfo::TypeArmada;
				}
			}
			else if( key == "Model")
			{
				if( val.find("Raspberry Pi 4") != string::npos )
				{
					this->type = SysInfo::TypeRPI4;
				}
				else if ( val.find("Raspberry Pi 3") != string::npos )
				{
					this->type = SysInfo::TypeRPI3;
				}
			}
		}
	}

    // Read override config to set type.
    // This must be done prior to setting the defaults in order to have "type" set.
    if( File::FileExists(DEVICEDBPATH) ) {
        string fil = File::GetContentAsString(DEVICEDBPATH);
        Json::Value db;

        if( Json::Reader().parse(fil, db) )
        {
            if( db.isMember("override") && db["override"].isObject() )
            {
                this->ParseExtEntry( db["override"]);
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
	// Setup system defaults

	// Setup sensible default values first.
	this->networkdevice = NetUtils::GetDefaultDevice();
	this->passworddevicepath = "Undefined";
	this->serialnbrdevice = "Undefined";
	this->backuprootpath = "/mnt/backup/";

	// Override with possible hardcoded values
	switch( this->type )
	{
	case TypeOpi:
		this->storagedevicepath = "/dev/disk/by-path";
		this->storagedevice = "platform-48060000.mmc";
		this->storagepartition = "-part1";
		this->passworddevicepath = "/dev/sda1";
		this->networkdevice = "eth0";
        this->serialnbrdevice = "/sys/bus/i2c/devices/0-0050/eeprom";
        break;
	case TypeArmada: // Keep
		this->storagedevicepath = "/dev/disk/by-path";
		this->storagedevice = "platform-f10a8000.sata-ata-2";
		this->storagepartition = "-part1";
		this->passworddevicepath = "/dev/disk/by-path/platform-f10f8000.usb3-usb-0:1:1.0-scsi-0:0:0:0-part1";
		this->networkdevice = "eth0";
		this->serialnbrdevice = "/sys/bus/i2c/devices/0-0057/eeprom";
		this->backuprootpath = "/mnt/backup/";
		break;
	default:
		break;
	}

	if( File::FileExists(DEVICEDBPATH) )
	{
		// Possibly override all above with external config
		this->ParseExtConfig();
	}
}

void SysInfo::ParseExtConfig()
{
	// Read override config
	string fil = File::GetContentAsString(DEVICEDBPATH);
	Json::Value db;

	if( Json::Reader().parse(fil, db) )
	{
		// First add any default settings from file
		if( db.isMember("default") && db["default"].isObject() )
		{
			this->ParseExtEntry(db["default"]);
		}

		list<string> devices({"opi","xu4","olimexa20","armada","pc","rpi3","rpi4"});

		for(const string& dev: devices )
		{
			if( db.isMember(dev) && db[dev].isObject() )
			{
				if( this->TypeFromName(dev) == this->Type()  )
				{
					this->ParseExtEntry( db[dev]);
				}
			}
		}

	}
}

void SysInfo::ParseExtEntry(Json::Value &v)
{
	if( v.isMember("storagedevicepath") && v["storagedevicepath"].isString() )
	{
		this->storagedevicepath = v["storagedevicepath"].asString();
	}
	if( v.isMember("storagedevice") && v["storagedevice"].isString() )
	{
		this->storagedevice = v["storagedevice"].asString();
	}
	if( v.isMember("storagepartition") && v["storagepartition"].isString() )
	{
		this->storagepartition = v["storagepartition"].asString();
	}
	if( v.isMember("passworddevicepath") && v["passworddevicepath"].isString() )
	{
		this->passworddevicepath = v["passworddevicepath"].asString();
	}
	if( v.isMember("networkdevice") && v["networkdevice"].isString() )
	{
		this->networkdevice = v["networkdevice"].asString();
	}
    if( v.isMember("serialnbrdevice") && v["serialnbrdevice"].isString() )
    {
        this->serialnbrdevice = v["serialnbrdevice"].asString();
    }
    if( v.isMember("type") && v["type"].isString() )
    {
        this->type = this->TypeFromName( v["type"].asString() );
    }
}

} // Namespace OPI
