#ifndef SYSINFO_H
#define SYSINFO_H


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
		TypeKeep,
		TypePC,
		TypeUnknown
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

	virtual ~SysInfo();

	static bool isOpi();
	static bool isKeep();
	static bool isPC();

private:
	void GuessType();
	void SetupPaths();
	int numcpus;
	SysType type;
	SysArch arch;

	string storagedevicepath;	// Path to device node with storage block devive (/dev)
	string storagedevice;		// Block device name (sdg,mmcblk0)
	string storagepartition;	// Partition on storage device (0,1,2)

	string passworddevicepath;	// Where to look for stored password

};

extern SysInfo sysinfo;

}
#endif // SYSINFO_H
