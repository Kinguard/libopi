#ifndef TESTHOSTSCONFIG_H
#define TESTHOSTSCONFIG_H

#include <cppunit/extensions/HelperMacros.h>


class TestHostsConfig: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestHostsConfig );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST_SUITE_END();


public:
	void setUp();
	void tearDown();

	void Test();

};

#endif // TESTHOSTSCONFIG_H
