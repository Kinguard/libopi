#ifndef TESTJSONHELPER_H_
#define TESTJSONHELPER_H_

#include <cppunit/extensions/HelperMacros.h>

class TestJsonHelper: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestJsonHelper );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST( TestCallback );
	CPPUNIT_TEST( TestConverters );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void Test();
	void TestCallback();
	void TestConverters();
};

#endif /* TESTJSONHELPER_H_ */
