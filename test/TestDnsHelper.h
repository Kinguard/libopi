#ifndef TESTDNSHELPER_H_
#define TESTDNSHELPER_H_

#include <cppunit/extensions/HelperMacros.h>

class TestDnsHelper: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestDnsHelper );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void Test();
};

#endif /* TESTDNSHELPER_H_ */
