#include "TestDiskHelper.h"

#include <unistd.h>
#include "DiskHelper.h"

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
