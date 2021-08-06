#ifndef TESTSERVICEHELPER_H_
#define TESTSERVICEHELPER_H_

#include <cppunit/extensions/HelperMacros.h>

class TestServiceHelper: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestServiceHelper );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void Test();
};

#endif /* TESTSERVICEHELPER_H_ */
