#include "TestServiceHelper.h"

#include "ServiceHelper.h"

using namespace OPI::ServiceHelper;

CPPUNIT_TEST_SUITE_REGISTRATION ( TestServiceHelper );


void TestServiceHelper::setUp()
{
}

void TestServiceHelper::tearDown()
{
}

void TestServiceHelper::Test()
{

	CPPUNIT_ASSERT( IsAvailable("ssh") );
	CPPUNIT_ASSERT( ! IsAvailable("ssaassh") );

}
