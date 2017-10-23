#ifndef TESTAUTHSERVER_H_
#define TESTAUTHSERVER_H_

#include <cppunit/extensions/HelperMacros.h>

class TestAuthServer: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestAuthServer );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST( Login );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void Test();
	void Login();
};

#endif /* TESTAUTHSERVER_H_ */
