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

#ifndef SYSINFO_H
#define SYSINFO_H

#include <json/json.h>

#include <string>

using namespace std;

namespace OPI
{

class SysInfo
{
public:

	enum SysType {
		TypeUndefined,
		TypeOpi,
		TypeXu4,
		TypeOlimexA20,
		TypeArmada,
		TypePC,
		TypeUnknown
	};

    std::vector<std::string> Domains {
        "",
        "op-i.me",
        "",
        "",
        "mykeep.net",
        "",
        ""
    };

    std::vector<std::string> SysTypeText {
        "Undefined",
        "Opi",
        "Xu4",
        "OlimexA20",
        "Armada",
        "PC",
        "Unknown"
    };
    enum SysArch {
		ArchUndefined,
		ArchArm,
		ArchIntel,
		ArchUnknown
	};

	SysInfo();

	int NumCpus();
	SysType Type();
	SysArch Arch();

	/**
	 * @brief StorageDevicePath Get complete path to storage device
	 * @return  path to storage device
	 */
	string StorageDevicePath();

	/**
	 * @brief StorageDevice Get path to storage block device (/dev/sdg)
	 * @return path to storage block device
	 */
	string StorageDevice();

	/**
	 * @brief StorageDeviceBlock Get storage device (sdg, mmcblk0)
	 * @return name of storage device
	 */
	string StorageDeviceBlock();

	/**
	 * @brief StorageDevicePartition Get storage partition (1, -part1)
	 * @return name of storage partition
	 */
	string StorageDevicePartition();

	/**
	 * @brief PasswordDevice Get complete path to password storage device
	 * @return path to password device
	 */
	string PasswordDevice();

	/**
	 * @brief NetworkDevice Get active network device to use for operation
	 * @return name of device i.e. "eth0"
	 */
	string NetworkDevice();

    /**
     * @brief SerialNumber Get the serial number from the eeprom
     * @return the serial number "1748KEEP1234"
     */
    string SerialNumber();
    string SerialNumberDevice();

    /**
     * @brief BackupRootPath Get base path on where remote FS for backup is mounted.
     * @return path to use for mount points, i.e. "/mnt/backup"
     */
    string BackupRootPath();


	virtual ~SysInfo();

	static bool isArmada();
	static bool isOpi();
	static bool isXu4();
	static bool isOlimexA20();
	static bool isPC();

	static bool useLVM();
	static bool useLUKS();

	static SysType TypeFromName(const string& devname);

private:
	void GuessType();
	void SetupPaths();
	void ParseExtConfig();
	void ParseExtEntry(Json::Value& v);
	int numcpus;
	SysType type;
    SysArch arch;

	string storagedevicepath;	// Path to device node with storage block devive (/dev)
	string storagedevice;		// Block device name (sdg,mmcblk0)
	string storagepartition;	// Partition on storage device (0,1,2)

	string passworddevicepath;	// Where to look for stored password

	string networkdevice;		// Which network device to use for operation
    string serialnbrdevice;		// Path to eeprom that holds the serial number
    string backuprootpath;      // Root path on where to mount remote FS for backup.
};

extern SysInfo sysinfo;

}
#endif // SYSINFO_H
