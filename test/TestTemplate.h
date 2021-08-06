#ifndef TESTXX_H_
#define TESTXX_H_

#include <cppunit/extensions/HelperMacros.h>

class TestXX: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestXX );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void Test();
};

#endif /* TESTXX_H_ */
