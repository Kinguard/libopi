
#include <libutils/Exceptions.h>
#include <libutils/FileUtils.h>
#include <libutils/Process.h>
#include <libutils/String.h>

#include <parted/parted.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <blkid.h>

#include <iostream>
#include <sstream>
#include <map>
#include <string>

#include "DiskHelper.h"

using namespace std;

namespace OPI {
namespace DiskHelper {


static bool do_stat(const std::string& path,mode_t mode )
{
	struct stat st = {};
	if(stat(path.c_str(),&st)){
		if( errno == ENOENT ){
				return false;
		}
		throw Utils::ErrnoException("Failed to check file");
	}
	return ((((st.st_mode)) & 0170000) & (mode));
}


void PartitionDevice(const string& device)
{
	PedDevice* dev = ped_device_get( device.c_str() );

	if( ! ped_device_open( dev ) )
	{
		throw runtime_error("Failed to open device");
	}

	PedDiskType* type = ped_disk_type_get( "msdos" );

	PedDisk* disk = ped_disk_new_fresh( dev, type );
	if( !disk )
	{
		throw runtime_error("Failed to create new partition table");
	}

	PedConstraint* constraint = ped_constraint_any( dev );
	PedGeometry* geom = ped_constraint_solve_max( constraint );

	PedPartition* part = ped_partition_new( disk, PED_PARTITION_NORMAL, nullptr, geom->start, geom->end );

	ped_geometry_destroy( geom );

	if( !part )
	{
		throw runtime_error("Failed to create new partition");
	}

	ped_exception_fetch_all();

	if( !ped_disk_add_partition( disk, part, constraint ) )
	{
		ped_exception_leave_all();
		throw runtime_error("Failed to add the new partition to the partition table");
	}

	ped_constraint_destroy( constraint );

	ped_exception_leave_all();


	ped_exception_catch();

	if (ped_partition_is_flag_available( part, PED_PARTITION_LBA ) )
	{
		ped_partition_set_flag( part, PED_PARTITION_LBA, 1 );
	}

	if (!ped_disk_commit_to_dev( disk ) )
	{
		throw runtime_error("Failed writing partition table to disk");
	}

	if (!ped_disk_commit_to_os( disk ) )
	{
		throw runtime_error("Inform kernel about the changes failed");
	}

	ped_disk_destroy( disk );

	if( ! ped_device_close( dev ) )
	{
		throw runtime_error("Failed closing device");
	}

}

void FormatPartition(const string& device, const string& label )
{
	string cmd="/sbin/mkfs -text4 -q -L"+label + " " + device;

	bool res = false;
	string errmsg;
	tie(res, errmsg) = Utils::Process::Exec(cmd);

	if( !res )
	{
		throw Utils::ErrnoException("Failed to format device ("+device+") errmsg ("+errmsg+")");
	}
}

void Mount(const string& device, const string& mountpoint, bool noatime, bool discard, const string &filesystem)
{
	stringstream ss;
	ss << "/bin/mount ";

	if( filesystem != "" )
	{
		ss << "-t"<<filesystem<< " ";
	}

	if( noatime && discard )
	{
		ss << "-o noatime,discard ";
	}
	else if( noatime )
	{
		ss << "-o noatime ";
	}
	else if( discard )
	{
		ss << "-o discard ";
	}

	ss << device << " " << mountpoint;

	bool res = false;
	string errmsg;
	tie(res, errmsg) = Utils::Process::Exec(ss.str());

	if( !res )
	{
		throw Utils::ErrnoException("Failed to mount "+device+" on "+mountpoint+ " errmsg ("+errmsg+")" );
	}
}

void Umount(const string& device)
{
	// TODO: Perhaps kill processes locking device using fuser
	string cmd = "/bin/umount "+device;

	bool res = false;
	string errmsg;
	tie(res, errmsg) = Utils::Process::Exec( cmd );

	if( !res )
	{
		throw Utils::ErrnoException("Failed to umount "+device+" ("+errmsg+")" );
	}
}

bool DeviceExists(const string &device)
{
	return do_stat(device, S_IFBLK );
}

size_t DeviceSize(const string &devicename)
{

	blkid_probe pr = blkid_new_probe_from_filename(devicename.c_str());

	if (!pr)
	{
		throw std::runtime_error(string("Failed to create blkid probe on ") + devicename);
	}

	size_t sz = blkid_probe_get_size( pr );

	blkid_free_probe(pr);

	return sz;
}


string IsMounted(const string &device)
{
	list<string> lines = Utils::File::GetContent( "/etc/mtab");

	string rdev;
	const string mapper ="/dev/mapper";
	if( device.compare(0,mapper.length(), mapper) == 0 )
	{
		// This is a mapper device that is mounted as its symlink
		// and not resolved namne, thus use provided name
		rdev = device;
	}
	else
	{
		rdev = Utils::File::RealPath(device);
	}

	map<string,string> tab;
	for( auto& line: lines)
	{
		list<string> words = Utils::String::Split(line);
		if( words.size() > 2 )
		{
			string device = words.front();
			words.pop_front();
			string mpoint = words.front();
			tab[device] = mpoint;
		}
	}

	return tab.find(rdev)==tab.end()?"":tab[rdev];
}

void SyncPaths(const string &src, const string &dst)
{
	string cmd = "/usr/bin/rsync -a "+src+" "+dst;

	bool res(false);
	tie(res, std::ignore) = Utils::Process::Exec( cmd );

	if( !res )
	{
		throw Utils::ErrnoException("Failed sync "+src+" with "+dst );
	}

}

static string getDiskName(const string& syspath)
{

	if( Utils::File::FileExists( syspath+"/device/model") )
	{
		return Utils::String::Trimmed(Utils::File::GetContentAsString(syspath+"/device/model"), " ");
	}
	else if( Utils::File::FileExists( syspath+"/device/name") )
	{
		return Utils::String::Trimmed(Utils::File::GetContentAsString(syspath+"/device/name"), " ");
	}
	return "N/A";
}

Json::Value StorageDevices()
{
	constexpr uint32_t BLOCKSIZE = 512;
	Json::Value ret;
	list<string> devs = Utils::File::Glob("/sys/class/block/*");

	for( auto& syspath: devs )
	{
		string dev = Utils::File::GetFileName(syspath);

		try
		{
			if( Utils::File::FileExists("/sys/class/block/"s + dev +"/partition") )
			{
				continue;
			}
			ret[dev]["syspath"]= syspath;
			ret[dev]["devpath"] = "/dev/"s + dev;
			ret[dev]["isphysical"] = Utils::File::LinkExists(syspath+"/device");

			if( ret[dev]["isphysical"].asBool() )
			{
				ret[dev]["model"] = getDiskName(syspath);
			}
			else
			{
				ret[dev]["model"] = "Virtual device";
			}

			uint64_t blocks = std::stol(Utils::File::GetContentAsString(syspath+"/size"));
			ret[dev]["blocks"] = Json::UInt64(blocks);
			ret[dev]["size"] = Json::UInt64(blocks * BLOCKSIZE);

			ret[dev]["removable"] = std::stoi(Utils::File::GetContentAsString(syspath+"/removable")) > 0;
			ret[dev]["readonly"] = std::stoi(Utils::File::GetContentAsString(syspath+"/ro")) > 0;

		}
		catch (std::exception& err)
		{
			cout << "Caught exception: " << err.what() << endl;

		}

	}

	return ret;
}

} // End NS
} // End NS
