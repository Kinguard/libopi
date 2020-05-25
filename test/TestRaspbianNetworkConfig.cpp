#include "TestRaspbianNetworkConfig.h"

#include "NetworkConfig.h"
#include "SysInfo.h"

#include <fstream>
#include <unistd.h>

#include <libutils/FileUtils.h>

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

	CPPUNIT_ASSERT_NO_THROW(NetUtils::RaspbianNetworkConfig(IFFILE_COPY));
	CPPUNIT_ASSERT_THROW(NetUtils::RaspbianNetworkConfig(IFFILE_COPY "NoNo"), std::runtime_error);

	{
		NetUtils::RaspbianNetworkConfig nc(IFFILE_COPY);

		CPPUNIT_ASSERT_THROW( nc.GetInterface("noifs"), std::runtime_error );

		Json::Value ifs = nc.GetInterfaces();

		CPPUNIT_ASSERT( (unsigned int)3 < ifs.size() );

		CPPUNIT_ASSERT( ifs.isMember("lo") );
		CPPUNIT_ASSERT( ifs.isMember("eth0") );
		CPPUNIT_ASSERT( ifs.isMember("eth1") );

		CPPUNIT_ASSERT_EQUAL( string("192.168.3.1"), ifs["eth1"]["options"]["address"][(uint)0].asString() );

	}

}

void TestRaspbianNetworkConfig::TestReload()
{

	{
		RaspbianNetworkConfig nc(IFFILE_COPY);
		Json::Value ifs = nc.GetInterfaces();
		CPPUNIT_ASSERT_EQUAL(string("static"), ifs["enp0s31f6"]["addressing"].asString() );
		nc.SetDHCP("enp0s31f6");
		nc.WriteConfig();
	}

	{
		RaspbianNetworkConfig nc(IFFILE_COPY);
		Json::Value ifs = nc.GetInterfaces();
		CPPUNIT_ASSERT_EQUAL(string("dhcp"), ifs["enp0s31f6"]["addressing"].asString() );
		nc.SetStatic("enp0s31f6", "10.0.0.1", "255.255.255.0", "1.1.1.1",{"1.2.3.4","4.5.6.7"});
		nc.WriteConfig();
	}


	{
		RaspbianNetworkConfig nc(IFFILE_COPY);
		Json::Value ifs = nc.GetInterfaces();
		CPPUNIT_ASSERT_EQUAL(string("static"), ifs["enp0s31f6"]["addressing"].asString() );
		CPPUNIT_ASSERT_EQUAL(string("10.0.0.1"), ifs["enp0s31f6"]["options"]["address"][(uint)0].asString() );
		CPPUNIT_ASSERT_EQUAL(string("255.255.255.0"),  ifs["enp0s31f6"]["options"]["netmask"][(uint)0].asString() );
		CPPUNIT_ASSERT_EQUAL(string("1.1.1.1"),  ifs["enp0s31f6"]["options"]["gateway"][(uint)0].asString() );

		CPPUNIT_ASSERT( ifs.isMember("lo") );
		CPPUNIT_ASSERT( ifs.isMember("eth0") );
		CPPUNIT_ASSERT( ifs.isMember("eth1") );
	}

}

