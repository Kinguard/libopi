#include "TestRaspbianNetworkConfig.h"

#include "NetworkConfig.h"
#include "SysInfo.h"

#include <fstream>
#include <unistd.h>

#include <iomanip>

#include <libutils/FileUtils.h>
#include <libutils/String.h>
CPPUNIT_TEST_SUITE_REGISTRATION ( TestRaspbianNetworkConfig );

#define IFFILE "./dhcpcd.conf"
#define IFFILE_COPY "./dhcpcd.conf.copy"

using namespace Utils;
using namespace OPI;
using namespace OPI::NetUtils;
using namespace std;

void TestRaspbianNetworkConfig::setUp()
{
	File::Copy(IFFILE, IFFILE_COPY);

}

void TestRaspbianNetworkConfig::tearDown()
{
	File::Delete(IFFILE_COPY);
}

void TestRaspbianNetworkConfig::Test()
{

	CPPUNIT_ASSERT_NO_THROW( NetUtils::RaspbianNetworkConfig(IFFILE_COPY) );
	CPPUNIT_ASSERT_THROW(NetUtils::RaspbianNetworkConfig(IFFILE_COPY "NoNo"), std::runtime_error);

	{
		NetUtils::RaspbianNetworkConfig nc(IFFILE_COPY);

		CPPUNIT_ASSERT_THROW( nc.GetInterface("noifs"), std::runtime_error );

		json ifs = nc.GetInterfaces();

		CPPUNIT_ASSERT( (unsigned int)3 < ifs.size() );

		CPPUNIT_ASSERT( ifs.contains("lo") );
		CPPUNIT_ASSERT( ifs.contains("eth0") );
		CPPUNIT_ASSERT( ifs.contains("eth1") );

		CPPUNIT_ASSERT_EQUAL( string("192.168.3.1"), ifs["eth1"]["options"]["address"][(uint)0].get<string>() );

	}

}

static void testequal(const string& addr)
{
	CPPUNIT_ASSERT_EQUAL(string(addr), NetUtils::IPv6Network(addr).asString() );
}


static void testequal(const string& expected, const string& given)
{
	CPPUNIT_ASSERT_EQUAL(expected, NetUtils::IPv6Network(given).asString() );
}


void TestRaspbianNetworkConfig::TestIPV6()
{

	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("1:2:3:4:5:6:7:8"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("1:2:3:4:5:6:7::"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("1:2:3:4:5:6::"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("1:2:3:4:5::"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("1:2:3:4::"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("1:2:3::"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("1:2::"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("1::"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("::"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("::2:3:4:5:6:7:8"));
	CPPUNIT_ASSERT_NO_THROW(NetUtils::IPv6Network("::3:4:5:6:7:8"));
	CPPUNIT_ASSERT_THROW(NetUtils::IPv6Network("1:2:3:4:5:6:7:8:9"),std::runtime_error);
	CPPUNIT_ASSERT_THROW(NetUtils::IPv6Network("1:2:3:4:5:6:7"),std::runtime_error);
	CPPUNIT_ASSERT_THROW(NetUtils::IPv6Network(""),std::runtime_error);

	testequal("1:2:3:4:5:6:7:8");
	testequal("1::4:5:6:7:8");
	testequal("1:2:0:4:5:6:7:8");
	testequal("ffe0::1");
	testequal("::1");

	testequal("1::4:5:0:0:8",	"1:0:0:4:5:0:0:8");
	testequal("1:2:3:4::",		"1:2:3:4:0:0:0:0");
	testequal("::5:6:7:8",		"0:0:0:0:5:6:7:8");
	testequal("ffff:fff:ff:f::1","ffff:0fff:00ff:000f:0:0:0:1");


	CPPUNIT_ASSERT_EQUAL(string("f000::"), NetUtils::IPv6Network(4).asString() );
	CPPUNIT_ASSERT_EQUAL(string("ff00::"), NetUtils::IPv6Network(8).asString() );
	CPPUNIT_ASSERT_EQUAL(string("fff0::"), NetUtils::IPv6Network(12).asString() );
	CPPUNIT_ASSERT_EQUAL(string("ffff::"), NetUtils::IPv6Network(16).asString() );
	CPPUNIT_ASSERT_EQUAL(string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), NetUtils::IPv6Network(128).asString() );

	for(int i = 0; i < 128; i++)
	{
		CPPUNIT_ASSERT_EQUAL((uint8_t)i, NetUtils::IPv6Network(i).asNetwork() );
	}


}

void TestRaspbianNetworkConfig::TestIPCheck()
{
	CPPUNIT_ASSERT( NetUtils::IsIPv4address("192.168.1.1"));
	CPPUNIT_ASSERT( !NetUtils::IsIPv4address("192.168.1.1.1"));

	CPPUNIT_ASSERT( NetUtils::IsIPv6address("::1"));
	CPPUNIT_ASSERT( !NetUtils::IsIPv6address(":::1"));
}

void TestRaspbianNetworkConfig::TestReload()
{
	if( true )
	{
		RaspbianNetworkConfig nc(IFFILE_COPY);
		nc.Dump();
		json ifs = nc.GetInterfaces();
		CPPUNIT_ASSERT_EQUAL(string("static"), ifs["enp0s31f6"]["addressing"].get<string>() );
		nc.SetDHCP("enp0s31f6");
		nc.Dump();
		nc.WriteConfig();
	}

	if( false )
	{
		RaspbianNetworkConfig nc(IFFILE_COPY);
		json ifs = nc.GetInterfaces();
		CPPUNIT_ASSERT_EQUAL(string("dhcp"), ifs["enp0s31f6"]["addressing"].get<string>() );
		nc.SetStatic("enp0s31f6", "10.0.0.1", "255.255.255.0", "1.1.1.1",{"1.2.3.4","4.5.6.7"});
		nc.WriteConfig();
	}


	if( false )
	{
		RaspbianNetworkConfig nc(IFFILE_COPY);
		json ifs = nc.GetInterfaces();
		CPPUNIT_ASSERT_EQUAL(string("static"), ifs["enp0s31f6"]["addressing"].get<string>() );
		CPPUNIT_ASSERT_EQUAL(string("10.0.0.1"), ifs["enp0s31f6"]["options"]["address"][(uint)0].get<string>() );
		CPPUNIT_ASSERT_EQUAL(string("255.255.255.0"),  ifs["enp0s31f6"]["options"]["netmask"][(uint)0].get<string>() );
		CPPUNIT_ASSERT_EQUAL(string("1.1.1.1"),  ifs["enp0s31f6"]["options"]["gateway"][(uint)0].get<string>() );

		CPPUNIT_ASSERT( ifs.contains("lo") );
		CPPUNIT_ASSERT( ifs.contains("eth0") );
		CPPUNIT_ASSERT( ifs.contains("eth1") );
	}

}

