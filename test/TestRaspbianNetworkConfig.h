#ifndef TESTNETWORKCONFIG_H_
#define TESTNETWORKCONFIG_H_

#include <cppunit/extensions/HelperMacros.h>

class TestRaspbianNetworkConfig: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestRaspbianNetworkConfig );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST( TestReload );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void Test();
	void TestReload();
};

#endif /* TESTNETWORKCONFIG_H_ */
