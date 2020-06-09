#ifndef TESTNETWORKCONFIG_H_
#define TESTNETWORKCONFIG_H_

#include <cppunit/extensions/HelperMacros.h>

class TestRaspbianNetworkConfig: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestRaspbianNetworkConfig );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST( TestIPV6 );
	CPPUNIT_TEST( TestReload );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void Test();
	void TestIPV6();
	void TestReload();
};

#endif /* TESTNETWORKCONFIG_H_ */
