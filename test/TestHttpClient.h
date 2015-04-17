#ifndef TESTHTTPCLIENT_H_
#define TESTHTTPCLIENT_H_

#include <cppunit/extensions/HelperMacros.h>

class TestHttpClient: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestHttpClient );
	CPPUNIT_TEST( TestNoCA );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void TestNoCA();
};

#endif /* TESTHTTPCLIENT_H_ */
