#include "TestSysConfig.h"

#include "SysConfig.h"
#include <unistd.h>

CPPUNIT_TEST_SUITE_REGISTRATION ( TestSysConfig );

#define TESTDB "testdb.json"

using namespace OPI;

void TestSysConfig::setUp()
{

}

void TestSysConfig::tearDown()
{
	unlink(TESTDB);
}

void TestSysConfig::TestCreate()
{
	CPPUNIT_ASSERT_NO_THROW( SysConfig cfg);
}

void TestSysConfig::TestAccess()
{
	SysConfig cfg(TESTDB);
	// Should fail
	//CPPUNIT_ASSERT_THROW(cfg.GetKeyAsString("a","b"), runtime_error);

	CPPUNIT_ASSERT_NO_THROW(cfg.PutKey("a", "b1", "1") );
	CPPUNIT_ASSERT_NO_THROW(cfg.PutKey("a", "b2", "2") );
	CPPUNIT_ASSERT_NO_THROW(cfg.RemoveKey("a", "b1") );
	CPPUNIT_ASSERT_THROW( cfg.GetKeyAsString("a","b1"), runtime_error );
}
