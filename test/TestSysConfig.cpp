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
	CPPUNIT_ASSERT_THROW(cfg.GetKeyAsString("a","b"), runtime_error);

	CPPUNIT_ASSERT( ! cfg.HasScope( "a") );
	CPPUNIT_ASSERT( ! cfg.HasKey( "a", "b1") );
	CPPUNIT_ASSERT_NO_THROW(cfg.PutKey("a", "b1", "1") );
	CPPUNIT_ASSERT( cfg.HasScope( "a") );
	CPPUNIT_ASSERT( cfg.HasKey( "a", "b1") );

	CPPUNIT_ASSERT_NO_THROW(cfg.PutKey("a", "b1", "1") );
	CPPUNIT_ASSERT_NO_THROW(cfg.PutKey("a", "b2", "2") );
	CPPUNIT_ASSERT_NO_THROW(cfg.RemoveKey("a", "b1") );
	CPPUNIT_ASSERT_THROW( cfg.GetKeyAsString("a","b1"), runtime_error );
}

void TestSysConfig::TestInt()
{
	SysConfig cfg(TESTDB);
	CPPUNIT_ASSERT( ! cfg.HasScope( "a") );
	CPPUNIT_ASSERT( ! cfg.HasKey( "a", "b") );

	CPPUNIT_ASSERT_NO_THROW( cfg.PutKey("a","b", 100) );
	CPPUNIT_ASSERT( cfg.HasKey( "a", "b") );

	CPPUNIT_ASSERT_EQUAL(100, cfg.GetKeyAsInt("a","b") );
	CPPUNIT_ASSERT_NO_THROW( cfg.RemoveKey("a","b") );
	CPPUNIT_ASSERT( ! cfg.HasKey( "a", "b") );
	CPPUNIT_ASSERT( ! cfg.HasScope( "a" ) );

}

void TestSysConfig::TestBool()
{
	SysConfig cfg(TESTDB);
	CPPUNIT_ASSERT( ! cfg.HasScope( "a") );
	CPPUNIT_ASSERT( ! cfg.HasKey( "a", "b") );

	CPPUNIT_ASSERT_NO_THROW( cfg.PutKey("a","b", true) );
	CPPUNIT_ASSERT( cfg.HasKey( "a", "b") );
	CPPUNIT_ASSERT_EQUAL(true, cfg.GetKeyAsBool("a","b") );
}

void TestSysConfig::TestList()
{
	SysConfig cfg(TESTDB);

	CPPUNIT_ASSERT_NO_THROW( cfg.PutKey("a","b",list<string>({"a","b","c"})) );

	list<string> l = cfg.GetKeyAsStringList("a","b");
	CPPUNIT_ASSERT_EQUAL( 3, (int)l.size() );
	CPPUNIT_ASSERT_EQUAL( string("a"), l.front() );
	CPPUNIT_ASSERT_EQUAL( string("c"), l.back() );

	CPPUNIT_ASSERT_NO_THROW( cfg.PutKey("a","c", list<int>({1,2,3}) ) );
	list<int> il2 = cfg.GetKeyAsIntList( "a","c" );
	CPPUNIT_ASSERT_EQUAL( 3, (int)il2.size() );
	CPPUNIT_ASSERT_EQUAL( 1, il2.front() );
	CPPUNIT_ASSERT_EQUAL( 3, il2.back() );

	CPPUNIT_ASSERT_NO_THROW( cfg.PutKey("b","d", list<bool>({true,false,false}) ) );
	list<bool> bl = cfg.GetKeyAsBoolList( "b","d" );
	CPPUNIT_ASSERT_EQUAL( 3, (int)bl.size() );
	CPPUNIT_ASSERT_EQUAL( true, bl.front() );
	CPPUNIT_ASSERT_EQUAL( false, bl.back() );


	CPPUNIT_ASSERT_THROW( cfg.GetKeyAsStringList("b","d"), runtime_error);
	CPPUNIT_ASSERT_THROW( cfg.GetKeyAsIntList("a","b"), runtime_error);
	CPPUNIT_ASSERT_THROW( cfg.GetKeyAsBoolList("a","a"), runtime_error);
}
