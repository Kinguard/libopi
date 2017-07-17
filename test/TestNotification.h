#ifndef TESTNOTIFICATION_H_
#define TESTNOTIFICATION_H_

#include <cppunit/extensions/HelperMacros.h>

class TestNotification: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestNotification );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void Test();
};

#endif /* TESTNOTIFICATION_H_ */
