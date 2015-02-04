#ifndef TESTBACKUPHELPER_H_
#define TESTBACKUPHELPER_H_

#include <cppunit/extensions/HelperMacros.h>

class TestBackupHelper: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestBackupHelper );
	CPPUNIT_TEST( Test );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void Test();
};

#endif /* TESTBACKUPHELPER_H_ */
