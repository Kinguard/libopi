#ifndef DISKHELPER_H
#define DISKHELPER_H

#include <string>

#include <json/json.h>

using namespace std;

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

void PartitionDevice(const string& device);

void FormatPartition(const string& device, const string& label );

void Mount(const string& device, const string& mountpoint, bool noatime=true, bool discard=true, const string& filesystem = "ext4");

void Umount(const string& device);

void SyncPaths(const string& src, const string& dst);

/**
 * @brief StorageDevice retrieve storage device pointed out by devname
 * @param devname name of device as listed under /sys/class/block
 * @param ignorepartition if true return null if this is a partition
 * @return Json object with device information
 */
Json::Value StorageDevice(const string& devname, bool ignorepartition = false);

/**
 * @brief StorageDevices retrieve all known storage devices on system
 * @return Json array with device information
 */
Json::Value StorageDevices();

} // End NS

} // End NS
#endif // DISKHELPER_H
