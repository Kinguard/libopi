#ifndef TESTSYSINFO_H_
#define TESTSYSINFO_H_

#include <cppunit/extensions/HelperMacros.h>

class TestSysInfo: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestSysInfo );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST( TestNetworkDevice );
    CPPUNIT_TEST( TestSerialNumber );
    CPPUNIT_TEST( TestBackupRootPath );
	CPPUNIT_TEST( TestDeviceDB );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void Test();
	void TestNetworkDevice();
    void TestSerialNumber();
    void TestBackupRootPath();
	void TestDeviceDB();
};

#endif /* TESTSYSINFO_H_ */
