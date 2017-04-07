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

	virtual ~SysInfo();

	static bool isArmada();
	static bool isOpi();
	static bool isXu4();
	static bool isOlimexA20();
	static bool isPC();

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
};

extern SysInfo sysinfo;

}
#endif // SYSINFO_H
