#include "TestNotification.h"

#include "Notification.h"

#include <iostream>

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION ( TestNotification );

void TestNotification::setUp()
{
}

void TestNotification::tearDown()
{
}

void TestNotification::Test()
{
	CPPUNIT_ASSERT_NO_THROW( OPI::notification.Notify( OPI::Notification::Completed, "We are done!") );

	OPI::notification.Notify( OPI::Notification::Waiting, "We are waiting.." );
	OPI::notification.Notify( OPI::Notification::Completed, "We are totally done" );
	OPI::notification.Notify( OPI::Notification::Error, "We have failed!" );
}

/*
void TestSysInfo::TestNetworkDevice()
{
	switch( OPI::sysinfo.Type() )
	{
	case OPI::SysInfo::TypeArmada:
		CPPUNIT_ASSERT_EQUAL( OPI::sysinfo.NetworkDevice(), string("eth0") );
		break;
	case OPI::SysInfo::TypeOlimexA20:
		CPPUNIT_ASSERT_EQUAL( OPI::sysinfo.NetworkDevice(), string("eth0") );
		break;
	case OPI::SysInfo::TypeOpi:
		CPPUNIT_ASSERT_EQUAL( OPI::sysinfo.NetworkDevice(), string("eth0") );
		break;
	case OPI::SysInfo::TypePC:
		CPPUNIT_ASSERT_EQUAL( OPI::sysinfo.NetworkDevice(), string("eth0") );
		break;
	case OPI::SysInfo::TypeXu4:
		CPPUNIT_ASSERT_EQUAL( OPI::sysinfo.NetworkDevice(), string("eth0") );
		break;
	default:
		CPPUNIT_FAIL("Missing network device for unknown type");
	}
}
*/
