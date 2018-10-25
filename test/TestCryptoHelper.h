#ifndef TESTCRYPTOHELPER_H_
#define TESTCRYPTOHELPER_H_

#include <cppunit/extensions/HelperMacros.h>

class TestCryptoHelper: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestCryptoHelper );
	CPPUNIT_TEST( TestSelfSigned );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();
	void TestSelfSigned();
};

#endif /* TESTCRYPTOHELPER_H_ */
