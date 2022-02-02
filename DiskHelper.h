#ifndef DISKHELPER_H
#define DISKHELPER_H

#include <string>
#include <list>

#include <nlohmann/json.hpp>

using namespace std;

using json = nlohmann::json;

namespace OPI {
namespace DiskHelper {

bool DeviceExists( const string& device);

size_t DeviceSize( const string& devicename);

/**
 * @brief IsMounted check if device is mounted
 * @param device - physical devicepath
 * @return  mountpoint if mounted otherwise empty string
 */
string IsMounted( const string& device);

list<string> MountPoints( const string& device );

void PartitionDevice(const string& device);

void FormatPartition(const string& device, const string& label );

/**
 * @brief PartitionName try intelligently guess a partition name for device
 * @param devicename "base" name of device or devicepath i.e. sda or /dev/sda
 * @param partno partition number
 * @return guessed partition name for partition partno on device
 */
string PartitionName(const string& devicename, uint partno=1);

void Mount(const string& device, const string& mountpoint, bool noatime=true, bool discard=true, const string& filesystem = "ext4");

void Umount(const string& device);

void SyncPaths(const string& src, const string& dst);

/**
 * @brief StorageDevice retrieve storage device pointed out by devname
 * @param devname name of device as listed under /sys/class/block
 * @param ignorepartition if true return null if this is a partition
 * @return Json object with device information
 */
json StorageDevice(const string& devname, bool ignorepartition = false);

/**
 * @brief StorageDevices retrieve all known storage devices on system
 * @return Json array with device information
 */
json StorageDevices();

/**
 * @brief StatFs get storage info on mounted filesystem
 *		  (Wrapper around statvfs(3))
 * @param path string path to any location in mounted fs
 * @return Json value with:
 *		"block_size",	native size of a block
 *		"fragment_size", filesystem fragment size
 *		"blocks_total", filssystem size in fragments
 *		"blocks_free", filesystem free in fragments
 */
json StatFs(const string& path);

} // End NS

} // End NS
#endif // DISKHELPER_H
