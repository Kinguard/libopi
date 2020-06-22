#include "TestHostsConfig.h"

#include "HostsConfig.h"

#include <iostream>

using namespace  std;
using namespace  OPI;
CPPUNIT_TEST_SUITE_REGISTRATION ( TestHostsConfig );


void TestHostsConfig::setUp()
{
}

void TestHostsConfig::tearDown()
{
}

void TestHostsConfig::Test()
{

	// Create

	CPPUNIT_ASSERT_NO_THROW(HostsConfig());

	HostsConfig conf;

	OPI::HostEntryPtr e = HostEntry::MakeShared();

	e->hostname="ett-test";
	e->address = shared_ptr<NetUtils::IPNetwork>(new NetUtils::IPv4Network("1.1.1.1"));
	e->aliases = {"a1", "a2"};

	// Add by entry
	conf.AddEntry(e);

	HostEntryPtr p = conf.GetEntry("ett-test");
	CPPUNIT_ASSERT( p != nullptr );
	CPPUNIT_ASSERT_EQUAL( string("ett-test"), p->hostname);
	CPPUNIT_ASSERT_EQUAL( (size_t)2, p->aliases.size());

	// Update
	p->hostname = "changed";
	p->aliases.push_back("b3");
	conf.UpdateEntry(p);

	e = conf.GetEntry("changed");
	CPPUNIT_ASSERT( e != nullptr );
	CPPUNIT_ASSERT_EQUAL( string("changed"), e->hostname);
	CPPUNIT_ASSERT_EQUAL( (size_t)3, e->aliases.size());

	// Add by strings
	conf.AddEntry("192.168.3.1", "another", {"al1", "al2", "al3"});

	// Get by address as string
	p = conf.GetEntryByAddress("192.168.3.1");
	CPPUNIT_ASSERT( p != nullptr );
	CPPUNIT_ASSERT_EQUAL( string("another"), p->hostname);
	CPPUNIT_ASSERT_EQUAL( (size_t)3, p->aliases.size());

	// Get by network
	e = conf.GetEntry(
				NetUtils::IPNetworkPtr(new NetUtils::IPv4Network("1.1.1.1"))
				);
	CPPUNIT_ASSERT( e != nullptr );
	CPPUNIT_ASSERT_EQUAL( string("changed"), e->hostname);
	CPPUNIT_ASSERT_EQUAL( (size_t)3, e->aliases.size());

	// Delete
	conf.DeleteEntry( e );
	p = conf.GetEntryByAddress("1.1.1.1");
	CPPUNIT_ASSERT( p == nullptr );

	//conf.Dump();
}
