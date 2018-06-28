#ifndef TESTSYSCONFIG_H
#define TESTSYSCONFIG_H

#include <cppunit/extensions/HelperMacros.h>


class TestSysConfig: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestSysConfig );
	CPPUNIT_TEST( TestCreate );
	CPPUNIT_TEST( TestAccess );
	CPPUNIT_TEST( TestReadWrite );
	CPPUNIT_TEST( TestInt );
	CPPUNIT_TEST( TestBool );
	CPPUNIT_TEST( TestList );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void TestCreate();
	void TestAccess();
	void TestReadWrite();
	void TestInt();
	void TestBool();
	void TestList();
};

#endif // TESTSYSCONFIG_H
