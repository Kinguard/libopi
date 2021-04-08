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
	for( auto line: lines)
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

void TestDiskHelper::TestStorageDevices()
{
	auto disks = OPI::DiskHelper::StorageDevices();
	CPPUNIT_ASSERT(disks.size() > 0 );
}
