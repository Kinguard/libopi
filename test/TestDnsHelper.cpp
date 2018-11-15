#include "TestDnsHelper.h"

#include <unistd.h>
#include "DnsHelper.h"

CPPUNIT_TEST_SUITE_REGISTRATION ( TestDnsHelper );

using namespace OPI;

void TestDnsHelper::setUp()
{
}

void TestDnsHelper::tearDown()
{
}

void TestDnsHelper::Test()
{

	OPI::Dns::DnsHelper dh;

	CPPUNIT_ASSERT_NO_THROW(dh.Query("openproducts.com", Dns::QueryType::TXT));
	unsigned long am = dh.getQueries().size();
	CPPUNIT_ASSERT_EQUAL( 1, (int)am );
	am = dh.getAnswers().size();
	CPPUNIT_ASSERT_EQUAL( 1, (int)am );

}
