#include "TestSysInfo.h"

#include "SysInfo.h"

#include <iostream>

#include <libutils/FileUtils.h>
#include <libutils/Logger.h>

using namespace std;
using namespace Utils;

CPPUNIT_TEST_SUITE_REGISTRATION ( TestSysInfo );

void TestSysInfo::setUp()
{
#if 0
    cout << "\n"
	<< "Platform         : " << OPI::sysinfo.SysTypeText[OPI::sysinfo.Type() ] << endl
	<< "Numcpus          : " << OPI::sysinfo.NumCpus() << endl
	<< "Storagedevicepath: " << OPI::sysinfo.StorageDevicePath() << endl
	<< "Storagedevice    : " << OPI::sysinfo.StorageDevice() << endl
	<< "StorageBlockdev  : " << OPI::sysinfo.StorageDeviceBlock() << endl
	<< "Storagepartition : " << OPI::sysinfo.StorageDevicePartition() << endl
	<< "NetworkDevice    : " << OPI::sysinfo.NetworkDevice() << endl
    << "Password device  : " << OPI::sysinfo.PasswordDevice() << endl
    << "SerialNbr device : " << OPI::sysinfo.SerialNumberDevice() << endl;
#endif
}

void TestSysInfo::tearDown()
{
}

void TestSysInfo::Test()
{

	switch( OPI::sysinfo.Type() )
	{
	case OPI::SysInfo::TypeArmada:
		CPPUNIT_ASSERT( OPI::SysInfo::isArmada() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isOpi() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isXu4() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isOlimexA20() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isPC() );
		break;
	case OPI::SysInfo::TypeOlimexA20:
		CPPUNIT_ASSERT( ! OPI::SysInfo::isArmada() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isOpi() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isXu4() );
		CPPUNIT_ASSERT( OPI::SysInfo::isOlimexA20() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isPC() );
		break;
	case OPI::SysInfo::TypeOpi:
		CPPUNIT_ASSERT( ! OPI::SysInfo::isArmada() );
		CPPUNIT_ASSERT( OPI::SysInfo::isOpi() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isXu4() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isOlimexA20() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isPC() );
		break;
	case OPI::SysInfo::TypePC:
		CPPUNIT_ASSERT( ! OPI::SysInfo::isArmada() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isOpi() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isXu4() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isOlimexA20() );
		CPPUNIT_ASSERT( OPI::SysInfo::isPC() );
		break;
	case OPI::SysInfo::TypeXu4:
		CPPUNIT_ASSERT( ! OPI::SysInfo::isArmada() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isOpi() );
		CPPUNIT_ASSERT( OPI::SysInfo::isXu4() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isOlimexA20() );
		CPPUNIT_ASSERT( ! OPI::SysInfo::isPC() );
		break;
	default:
		CPPUNIT_FAIL("Unknown system type");
	}

}

void TestSysInfo::TestNetworkDevice()
{
	switch( OPI::sysinfo.Type() )
	{
	case OPI::SysInfo::TypeArmada:
        CPPUNIT_ASSERT_EQUAL( string("eth0"), OPI::sysinfo.NetworkDevice() );
		break;
	case OPI::SysInfo::TypeOlimexA20:
        CPPUNIT_ASSERT_EQUAL( string("eth0"), OPI::sysinfo.NetworkDevice());
		break;
	case OPI::SysInfo::TypeOpi:
        CPPUNIT_ASSERT_EQUAL( string("eth0"), OPI::sysinfo.NetworkDevice() );
		break;
	case OPI::SysInfo::TypePC:
        CPPUNIT_ASSERT_EQUAL( string("enp0s31f6"), OPI::sysinfo.NetworkDevice() );
		break;
	case OPI::SysInfo::TypeXu4:
        CPPUNIT_ASSERT_EQUAL( string("eth0"), OPI::sysinfo.NetworkDevice() );
		break;
	default:
		CPPUNIT_FAIL("Missing network device for unknown type");
	}
}

void TestSysInfo::TestSerialNumber()
{

    printf("\nSERIAL: %s\n",OPI::sysinfo.SerialNumber().c_str());

    switch( OPI::sysinfo.Type() )
    {
    case OPI::SysInfo::TypeOpi:
        CPPUNIT_ASSERT( OPI::sysinfo.SerialNumber().length() == 12 );
        break;
    case OPI::SysInfo::TypeArmada:
        CPPUNIT_ASSERT( OPI::sysinfo.SerialNumber().length() == 12 );
        break;
    case OPI::SysInfo::TypeOlimexA20:
    case OPI::SysInfo::TypePC:
    case OPI::SysInfo::TypeXu4:
        CPPUNIT_ASSERT_EQUAL(  string("Undefined"),OPI::sysinfo.SerialNumber() );
        break;
    default:
        CPPUNIT_FAIL("Missing Serial Number for unknown type");
    }
}

void TestSysInfo::TestBackupRootPath()
{
	CPPUNIT_ASSERT_EQUAL( string("/mnt/backup/"),OPI::sysinfo.BackupRootPath() );
}

#define DEVICEDBPATH	"/etc/opi/devicedb.json"

// Currently no actual tests implemented. Only dump some selected values for development
void TestSysInfo::TestDeviceDB()
{
	if( ! File::FileExists( DEVICEDBPATH) )
	{
		logg << Logger::Debug << "No devicedb found skipping test" << lend;
		return;
	}
	cout << endl;
	cout << "Typ     :" << OPI::sysinfo.SysTypeText[OPI::sysinfo.Type() ]  << endl;
	cout << "Password:" << OPI::sysinfo.PasswordDevice()  << endl;

}
