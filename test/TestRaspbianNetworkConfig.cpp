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


	NetUtils::RaspbianNetworkConfig rcfg(IFFILE_COPY);

	rcfg.Dump();

	rcfg.SetDHCP("eth0");
	rcfg.Dump();

	rcfg.SetStatic("eth0","192.168.1.12","255.255.0.0", "192.168.1.1",{"4.4.4.4","8.8.8.8"});

	rcfg.Dump();

	rcfg.WriteConfig();


	list<string> ifs = NetUtils::GetInterfaces();

	for(const auto& inf: ifs)
	{
		cout << "Interface: " << inf << "\n";
	}


	return;


	CPPUNIT_FAIL("AUTOFAIL");

	CPPUNIT_ASSERT_THROW( NetUtils::DebianNetworkConfig(IFFILE), std::runtime_error );

	CPPUNIT_ASSERT_NO_THROW( DebianNetworkConfig(IFFILE ".1") );
	CPPUNIT_ASSERT_NO_THROW( DebianNetworkConfig(IFFILE ".2") );

	{
		DebianNetworkConfig nc(IFFILE ".1");
		//nc.Dump();
		nc.WriteConfig();

		CPPUNIT_ASSERT_THROW( nc.GetInterface("noifs"), std::runtime_error );

		Json::Value ifs = nc.GetInterfaces();

		CPPUNIT_ASSERT_EQUAL( (unsigned int)3, ifs.size() );

		CPPUNIT_ASSERT( ifs.isMember("lo") );
		CPPUNIT_ASSERT( ifs.isMember("eth0") );
		CPPUNIT_ASSERT( ifs.isMember("eth1") );

		CPPUNIT_ASSERT( ifs["lo"]["auto"].asBool() );
		CPPUNIT_ASSERT( ifs["eth0"]["auto"].asBool() );
		CPPUNIT_ASSERT( ! ifs["eth1"]["auto"].asBool() );

		CPPUNIT_ASSERT_EQUAL( string("192.168.1.122"), ifs["eth1"]["options"]["address"][(uint)0].asString() );

	}


	{
		DebianNetworkConfig nc(IFFILE ".2");
		//nc.Dump();
		nc.WriteConfig();
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
		cout << "Write interfaces:\n"<< ifs.toStyledString()<<endl;
		cout << "Adress:" << ifs["enp0s31f6"]["addressing"].asString() << endl;
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

