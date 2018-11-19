#include "TestJsonHelper.h"

#include <unistd.h>
#include "JsonHelper.h"

CPPUNIT_TEST_SUITE_REGISTRATION ( TestJsonHelper );

using namespace OPI;
using namespace JsonHelper;


#define CHK_A	0x00000001
#define CHK_B	0x00000002
#define CHK_C	0x00000004
#define CHK_D	0x00000008
#define CHK_E	0x00000010
#define CHK_F	0x00000020

static const vector<TypeChecker::Check> checks =
{
	{ CHK_A, "a",	TypeChecker::Type::STRING },
	{ CHK_B, "b",	TypeChecker::Type::INT },
	{ CHK_C, "c",	TypeChecker::Type::BOOL },
	{ CHK_D, "d",	TypeChecker::Type::STRING },
	{ CHK_E, "e",	TypeChecker::Type::STRING },
	{ CHK_F, "f",	TypeChecker::Type::STRING },
};

void TestJsonHelper::setUp()
{

}

void TestJsonHelper::tearDown()
{
}

void TestJsonHelper::Test()
{

	TypeChecker tc(checks);
	Json::Value v;

	v["c"] = true;

	// Check type
	v["a"] = "Hello world";
	CPPUNIT_ASSERT(tc.Verify(CHK_A|CHK_C, v));
	v["a"]=10;
	CPPUNIT_ASSERT( !tc.Verify(CHK_A, v) );
	v["a"]=false;
	CPPUNIT_ASSERT( !tc.Verify(CHK_A, v) );

	// Check missing value
	CPPUNIT_ASSERT( !tc.Verify(CHK_D, v) );
}

void TestJsonHelper::TestCallback()
{
	bool called = false;
	TypeChecker tc(checks,
				   [&called](const string& msg, void *data)
	{
		(void) data;
		(void) msg;
		called = true;
		//cout << "Callback: " << msg << endl;
	}
				   );
	Json::Value v;
	// Should fail
	CPPUNIT_ASSERT(!tc.Verify(CHK_A, v));
	CPPUNIT_ASSERT(called);
}
