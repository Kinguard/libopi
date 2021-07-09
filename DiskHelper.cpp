
#include <libutils/Exceptions.h>
#include <libutils/FileUtils.h>
#include <libutils/Process.h>
#include <libutils/String.h>

#include <parted/parted.h>

#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <blkid.h>

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <tuple>

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
	list<string> lines = DiskHelper::MountPoints(device);

	if( lines.size() == 0 )
	{
		return "";
	}
	return lines.back();
}

list<string> MountPoints(const string &device)
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

	map<string,list<string>> tab;
	for( auto& line: lines)
	{
		list<string> words = Utils::String::Split(line);
		if( words.size() > 2 )
		{
			string device = words.front();
			words.pop_front();
			string mpoint = words.front();
			tab[device].emplace_back( mpoint );
		}
	}

	if( tab.find(rdev) != tab.end() )
	{
		return tab[rdev];
	}
	return {};
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


static string getLVMPath(const string& devname)
{
	list<string> devs = Utils::File::Glob("/dev/pool/*");
	for( const auto& dev : devs)
	{
		if( Utils::File::RealPath(dev) == "/dev/"s + devname )
		{
			return dev;
		}
	}
	return "";
}

static string getLUKSPath(const string& devname)
{
	list<string> devs = Utils::File::Glob("/dev/mapper/*");
	for( const auto& dev : devs)
	{
		if( Utils::File::RealPath(dev) == "/dev/"s + devname )
		{
			return dev;
		}
	}
	return "";
}

static string getIDPath(const string& devname)
{
	list<string> devs = Utils::File::Glob("/dev/disk/by-path/*");
	for( const auto& dev : devs)
	{
		if( Utils::File::RealPath(dev) == "/dev/"s + devname )
		{
			return dev;
		}
	}
	return "";
}


static tuple<string,string> getDMType(const string& devname)
{
	string uuid = Utils::File::GetContentAsString("/sys/class/block/"s + devname+"/dm/uuid");

	constexpr int CRYPT_LEN=5;
	constexpr int LVM_LEN=3;
	if( uuid.compare(0,CRYPT_LEN,"CRYPT") == 0 )
	{
		return {"luks",getLUKSPath(devname)};
	}
	else if (uuid.compare(0,LVM_LEN,"LVM") == 0 )
	{
		return {"lvm", getLVMPath(devname)};
	}

	return {"unknown",""};
}


Json::Value StorageDevices()
{
	Json::Value ret;
	list<string> devs = Utils::File::Glob("/sys/class/block/*");

	for( auto& syspath: devs )
	{
		string dev = Utils::File::GetFileName(syspath);

		Json::Value disk = StorageDevice(dev, true);
		if( ! disk.isNull() )
		{
			ret[dev] = disk;
		}

	}

	return ret;
}

Json::Value StorageDevice(const string &devname, bool ignorepartition)
{
	constexpr uint32_t BLOCKSIZE = 512;
	string syspath = "/sys/class/block/"s + devname;
	Json::Value ret;
	try
	{
		ret["partition"] = Utils::File::FileExists( syspath +"/partition");

		if( ret["partition"].asBool() && ignorepartition )
		{
			return Json::nullValue;
		}

		if( !ret["partition"].asBool() )
		{
			ret["partitions"] = Json::arrayValue;
			list<string> parts = Utils::File::Glob( syspath+"?*");
			for(const auto& part: parts)
			{
				//cout << "Partition: " << Utils::File::GetFileName(part) << endl;
				ret["partitions"].append(DiskHelper::StorageDevice(Utils::File::GetFileName(part)));
			}
		}


		ret["devname"] = devname;
		ret["syspath"]= syspath;
		ret["devpath"] = "/dev/"s + devname;
		ret["isphysical"] = Utils::File::LinkExists(syspath+"/device");

		if( ret["isphysical"].asBool() )
		{
			ret["model"] = getDiskName(syspath);
			ret["devpath-by-path"] = getIDPath(devname);
		}
		else
		{
			if( ret["partition"].asBool() )
			{
				ret["model"] = "Partition";
				ret["devpath-by-path"] = getIDPath(devname);
			}
			else
			{
				ret["model"] = "Virtual device";
				ret["devpath-by-path"] = "";
			}
		}

		ret["dm"] = Utils::File::DirExists( syspath +"/dm");

		if( ret["dm"].asBool() )
		{
			string type, path;
			tie(type, path) = getDMType(devname);
			ret["dm-type"] = type;
			ret["dm-path"] = path;

		}
		else
		{
			ret["dm-type"] = "";
			ret["dm-path"] = "";
		}

		uint64_t blocks = std::stol(Utils::File::GetContentAsString(syspath+"/size"));
		ret["blocks"] = Json::UInt64(blocks);
		ret["size"] = Json::UInt64(blocks * BLOCKSIZE);

		if( ! ret["partition"].asBool() )
		{
			ret["removable"] = std::stoi(Utils::File::GetContentAsString(syspath+"/removable")) > 0;
		}
		ret["readonly"] = std::stoi(Utils::File::GetContentAsString(syspath+"/ro")) > 0;

		list<string> mountpoints = DiskHelper::MountPoints(ret["devpath"].asString());
		ret["mountpoint"]=Json::arrayValue;
		if(mountpoints.size() > 0)
		{
			for( const auto& mp : mountpoints)
			{
				ret["mountpoint"].append(mp);
			}
			ret["mounted"] = true;
		}
		else
		{
			ret["mounted"] = false;
		}

	}
	catch (std::exception& err)
	{
		cout << "Caught exception: " << err.what() << endl;
		return Json::nullValue;

	}

	return ret;
}

static string parsedevlinks(string devicename, uint partno)
{
	vector<string> parts;
	Utils::String::Split(devicename, parts, "/");
	if( parts[2] == "by-id" || parts[2] == "by-path")
	{
		stringstream pnam;
		pnam << devicename << "-part" << partno;
		return pnam.str();
	}

	// Assume this is a partitioned device already so return orignal name
	return devicename;
}

string PartitionName(const string &devicename, uint partno)
{
	stringstream ss;
	ss << devicename;

	string sut;

	// Full path or device?
	if( devicename.find('/') != string::npos )
	{

		if( devicename.compare(0,13, "/dev/disk/by-") == 0)
		{
			return parsedevlinks(devicename, partno);
		}

		sut = Utils::File::GetFileName(devicename);
	}
	else
	{
		sut = devicename;
	}

	if( sut.compare(0,2,"sd") == 0 )
	{
		ss<<partno;
	}
	else
	{
		ss << "p" << partno;
	}
	return ss.str();
}

/**
 * @brief StatFs get storage info on mounted filesystem
 *		  (Wrapper around statvfs(3))
 * @param path string path to any location in mounted fs
 * @return Json value with:
 *		"block_size",	native size of a block
 *		"fragment_tsize", filesystem fragment size
 *		"blocks_total", filssystem size in fragments
 *		"blocks_free", filesystem free in fragments
 */
Json::Value StatFs(const string &path)
{
	struct statvfs vf = {};

	if( statvfs(path.c_str(), &vf)  < 0)
	{
		throw Utils::ErrnoException("Failed to stat filesystem: "s+path);
	}

	Json::Value ret;
	ret["block_size"] = Json::UInt(vf.f_bsize);
	ret["fragment_size"] = Json::UInt(vf.f_frsize);
	ret["blocks_total"] = Json::UInt64(vf.f_blocks);
	ret["blocks_free"] = Json::UInt64(vf.f_bavail);

	return ret;
}

} // End NS
} // End NS
