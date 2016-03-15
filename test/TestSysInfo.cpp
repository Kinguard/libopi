#include "TestSysInfo.h"

#include "SysInfo.h"

#include <iostream>

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION ( TestSysInfo );

void TestSysInfo::setUp()
{
#if 0
	cout << "\n"
	<< "Numcpus          : " << OPI::sysinfo.NumCpus() << endl
	<< "Storagedevicepath: " << OPI::sysinfo.StorageDevicePath() << endl
	<< "Storagedevice    : " << OPI::sysinfo.StorageDevice() << endl
	<< "Storagepartition : " << OPI::sysinfo.StoragePartition() << endl
	<< "Password device  : " << OPI::sysinfo.PasswordDevice() << endl;
#endif
}

void TestSysInfo::tearDown()
{
}

void TestSysInfo::Test()
{
	CPPUNIT_ASSERT_EQUAL( OPI::SysInfo::TypePC, OPI::sysinfo.Type() );

	CPPUNIT_ASSERT( ! OPI::SysInfo::isKeep() );
	CPPUNIT_ASSERT( ! OPI::SysInfo::isOpi() );
	CPPUNIT_ASSERT( OPI::SysInfo::isPC() );
}
