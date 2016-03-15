#ifndef DISKHELPER_H
#define DISKHELPER_H

#include <string>

using namespace std;

namespace OPI {
namespace DiskHelper {

bool DeviceExists( const string& device);

size_t DeviceSize( const string& devicename);

string IsMounted( const string& device);

void PartitionDevice(const string& device);

void FormatPartition(const string& device, const string& label );

void Mount(const string& device, const string& mountpoint, bool noatime=true, bool discard=true, const string& filesystem = "ext4");

void Umount(const string& device);

void SyncPaths(const string& src, const string& dst);

} // End NS

} // End NS
#endif // DISKHELPER_H
