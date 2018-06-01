#ifndef TESTSYSCONFIG_H
#define TESTSYSCONFIG_H

#include <cppunit/extensions/HelperMacros.h>


class TestSysConfig: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestSysConfig );
	CPPUNIT_TEST( TestCreate );
	CPPUNIT_TEST( TestAccess );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void TestCreate();
	void TestAccess();
};

#endif // TESTSYSCONFIG_H
