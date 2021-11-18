#include "TestDiskHelper.h"

#include <unistd.h>
#include "DiskHelper.h"

#include <libutils/String.h>
#include <libutils/FileUtils.h>

#include <list>

using namespace std;
using namespace Utils;

CPPUNIT_TEST_SUITE_REGISTRATION ( TestDiskHelper );

void TestDiskHelper::setUp()
{
}

void TestDiskHelper::tearDown()
{
}

void TestDiskHelper::Test()
{
	// Have to be root to do this :|
	if( geteuid() == 0 )
	{
		CPPUNIT_ASSERT( OPI::DiskHelper::DeviceSize("/dev/sda") > 0 );
	}



}

void TestDiskHelper::TestIsMounted()
{
	// Get all mounted devices

	list<string> lines = Utils::File::GetContent( "/etc/mtab");
	// Device, mountpath
	map<string,string> tab;
	// Reverse table mountpath, device
	map<string,string> rtab;
	for( const auto& line: lines)
	{
		list<string> words = Utils::String::Split(line);
		if( words.size() > 2 )
		{
			string device = words.front();
			words.pop_front();
			string mpoint = words.front();
			tab[device] = mpoint;
			rtab[mpoint] = device;
		}
	}

	list<string> disks = File::Glob("/dev/disk/by-path/*");
	for( auto& disk: disks)
	{
		//cout << "Found disk: " << disk << " " << File::RealPath(disk) << endl;

		if( tab.find(File::RealPath(disk)) != tab.end() )
		{
			//cout << "Disk: " << disk << " is mounted" << endl;
			CPPUNIT_ASSERT( OPI::DiskHelper::IsMounted(disk) != "" );
			CPPUNIT_ASSERT( OPI::DiskHelper::IsMounted(File::RealPath(disk)) != "" );
		}
		else
		{
			//cout << "Disk: " << disk << " is NOT mounted" << endl;
			CPPUNIT_ASSERT( OPI::DiskHelper::IsMounted(disk) == "" );
			CPPUNIT_ASSERT( OPI::DiskHelper::IsMounted(File::RealPath(disk)) == "" );
		}

	}
}

// A lot duplicated from above, consider refactor these together
void TestDiskHelper::TestMountPoints()
{
	// Get all mounted devices

	list<string> lines = Utils::File::GetContent( "/etc/mtab");
	// Device, mountpath
	map<string,string> tab;
	// Reverse table mountpath, device
	map<string,string> rtab;
	for( const auto& line: lines)
	{
		list<string> words = Utils::String::Split(line);
		if( words.size() > 2 )
		{
			string device = words.front();
			words.pop_front();
			string mpoint = words.front();
			tab[device] = mpoint;
			rtab[mpoint] = device;
		}
	}

	list<string> disks = File::Glob("/dev/disk/by-path/*");
	for( auto& disk: disks)
	{
		//cout << "Found disk: " << disk << " " << File::RealPath(disk) << endl;

		if( tab.find(File::RealPath(disk)) != tab.end() )
		{
			//cout << "Disk: " << disk << " is mounted" << endl;
			CPPUNIT_ASSERT( OPI::DiskHelper::MountPoints(disk).back() != "" );
			CPPUNIT_ASSERT( OPI::DiskHelper::MountPoints(File::RealPath(disk)).back() != "" );
		}
		else
		{
			//cout << "Disk: " << disk << " is NOT mounted" << endl;
			CPPUNIT_ASSERT( OPI::DiskHelper::MountPoints(disk).size() == 0 );
			CPPUNIT_ASSERT( OPI::DiskHelper::MountPoints(File::RealPath(disk)).size() == 0 );
		}

	}
}

void TestDiskHelper::TestStorageDevices()
{
	auto disks = OPI::DiskHelper::StorageDevices();

#if 0
	for(const auto& disk: disks.getMemberNames())
	{
		cout << "\nDisk:" << disk << ":" << disks[disk].toStyledString() << endl;
	}
#endif
	CPPUNIT_ASSERT(disks.size() > 0 );
}

void TestDiskHelper::TestPartitionName()
{
	using namespace OPI::DiskHelper;
	CPPUNIT_ASSERT_EQUAL( PartitionName("sda"), "sda1"s );
	CPPUNIT_ASSERT_EQUAL( PartitionName("/dev/sda"), "/dev/sda1"s );
	CPPUNIT_ASSERT_EQUAL( PartitionName("/sys/class/block/sda"), "/sys/class/block/sda1"s );
	CPPUNIT_ASSERT_EQUAL( PartitionName("sda",2), "sda2"s );
	CPPUNIT_ASSERT_EQUAL( PartitionName("nvme0n1"), "nvme0n1p1"s );
	CPPUNIT_ASSERT_EQUAL( PartitionName("/dev/disk/by-path/platform-f10a8000.sata-ata-1"), "/dev/disk/by-path/platform-f10a8000.sata-ata-1-part1"s );
	CPPUNIT_ASSERT_EQUAL( PartitionName("/dev/disk/by-id/ata-KINGSTON_SUV400S37120G_50026B767300A676"),
						  "/dev/disk/by-id/ata-KINGSTON_SUV400S37120G_50026B767300A676-part1"s );

	CPPUNIT_ASSERT_EQUAL( PartitionName("/dev/disk/by-label/root"),
						  "/dev/disk/by-label/root"s );

	CPPUNIT_ASSERT_EQUAL( PartitionName("/dev/disk/by-uuid/73d88418-1a7c-4459-bb5c-27fdb291fa49"),
						  "/dev/disk/by-uuid/73d88418-1a7c-4459-bb5c-27fdb291fa49"s );



	//Fail tests
	//CPPUNIT_ASSERT_EQUAL( PartitionName("sda"), "sda2"s );
	//CPPUNIT_ASSERT_EQUAL( PartitionName("sda"), "sda4"s );
}


void TestDiskHelper::TestFilesystemInfo()
{
	json v = OPI::DiskHelper::StatFs("/");
//	cout << v.toStyledString() << endl;

	CPPUNIT_ASSERT_NO_THROW( v = OPI::DiskHelper::StatFs("/") );
	CPPUNIT_ASSERT( v.contains("block_size") );
	CPPUNIT_ASSERT( v.contains("fragment_size") );
	CPPUNIT_ASSERT( v.contains("blocks_total") );
	CPPUNIT_ASSERT( v.contains("blocks_free") );

	CPPUNIT_ASSERT( v["block_size"].get<uint>() > 0 );
	CPPUNIT_ASSERT( v["fragment_size"].get<uint>() > 0 );
	CPPUNIT_ASSERT( v["blocks_total"].get<uint64_t>() > 0 );
	CPPUNIT_ASSERT( v["blocks_free"].get<uint64_t>() > 0 );

	CPPUNIT_ASSERT_THROW(OPI::DiskHelper::StatFs("DUMMYVALUE"), Utils::ErrnoException);
}

