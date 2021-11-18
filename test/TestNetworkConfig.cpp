#include "TestNetworkConfig.h"

#include "NetworkConfig.h"
#include "SysInfo.h"

#include <fstream>
#include <unistd.h>

#include <libutils/FileUtils.h>

CPPUNIT_TEST_SUITE_REGISTRATION ( TestNetworkConfig );

#define IFFILE "interfaces"

using namespace Utils;
using namespace OPI;
using namespace OPI::NetUtils;
using namespace std;

void TestNetworkConfig::setUp()
{
	{
		ofstream of(IFFILE ".1");
		of << "# This is a comment\n"
			  "  auto lo eth0\n"
			  "iface eth0 inet dhcp  \n"
			  "iface eth1 inet static \n"
			  "\taddress 192.168.1.122\n"
			  "\tgateway 192.168.1.1\n"
			  "\tnetmask 255.255.255.0\n"
			  "iface lo inet loopback\n"<< endl;
		of.close();
	}

	{
		ofstream of(IFFILE ".2");
		of << "# This is a comment\n"
			  "  auto lo eth0\n"
			  "iface eth0 inet static  \n"
			  "\taddress 192.168.1.122\n"
			  "\tgateway 192.168.1.1\n"
			  "\tnetmask 255.255.255.0\n"
			  "iface lo inet loopback\n"<< endl;
		of.close();
	}
}

void TestNetworkConfig::tearDown()
{
	if( unlink(IFFILE ".1") != 0)
	{
		cerr << "Failed to erase file"<<endl;
	}

	if( unlink(IFFILE ".2") != 0)
	{
		cerr << "Failed to erase file"<<endl;
	}
}

static void dumpfile(const string& file)
{
	(void) file;
	//cout << "File: " << file << "\n"<<Utils::File::GetContentAsString(file,true)<<endl;;
}

void TestNetworkConfig::Test()
{
	CPPUNIT_ASSERT_THROW( NetUtils::DebianNetworkConfig(IFFILE), std::runtime_error );

	CPPUNIT_ASSERT_NO_THROW( DebianNetworkConfig(IFFILE ".1") );
	CPPUNIT_ASSERT_NO_THROW( DebianNetworkConfig(IFFILE ".2") );

	{
		DebianNetworkConfig nc(IFFILE ".1");
		//nc.Dump();
		nc.WriteConfig();
		dumpfile(IFFILE".1");

		CPPUNIT_ASSERT_THROW( nc.GetInterface("noifs"), std::runtime_error );

		json ifs = nc.GetInterfaces();

		// File contains three interfaces but additional interfaces might exist on system
		CPPUNIT_ASSERT( ifs.size() >= 3 );

		CPPUNIT_ASSERT( ifs.contains("lo") );
		CPPUNIT_ASSERT( ifs.contains("eth0") );
		CPPUNIT_ASSERT( ifs.contains("eth1") );

		CPPUNIT_ASSERT( ifs["lo"]["auto"].get<bool>() );
		CPPUNIT_ASSERT( ifs["eth0"]["auto"].get<bool>() );
		CPPUNIT_ASSERT( ! ifs["eth1"]["auto"].get<bool>() );

		CPPUNIT_ASSERT_EQUAL( string("192.168.1.122"), ifs["eth1"]["options"]["address"][(uint)0].get<string>() );
	}


	{
		DebianNetworkConfig nc(IFFILE ".2");
		//nc.Dump();
		nc.WriteConfig();
		dumpfile(IFFILE".2");
	}
}

void TestNetworkConfig::TestReload()
{
#if 0
	if( true )
	{
		DebianNetworkConfig nc(IFFILE ".1");
		json ifs = nc.GetInterfaces();
		//CPPUNIT_ASSERT_EQUAL(string("static"), ifs["eth1"]["addressing"].get<string>() );
		//nc.SetDHCP("eth1");
		nc.WriteConfig();
		dumpfile(IFFILE".1");
	}

	if( false )
	{
		DebianNetworkConfig nc(IFFILE ".1");
		json ifs = nc.GetInterfaces();
		CPPUNIT_ASSERT_EQUAL(string("dhcp"), ifs["eth1"]["addressing"].get<string>() );
		nc.SetStatic("eth1", "10.0.0.1", "1.2.3.4", "1.1.1.1");
		nc.WriteConfig();
		dumpfile(IFFILE".1");
	}

	if( false )
	{
		DebianNetworkConfig nc(IFFILE ".1");
		json ifs = nc.GetInterfaces();
		CPPUNIT_ASSERT_EQUAL(string("static"), ifs["eth1"]["addressing"].get<string>() );
		CPPUNIT_ASSERT_EQUAL(string("10.0.0.1"), ifs["eth1"]["options"]["address"][(uint)0].get<string>() );
		CPPUNIT_ASSERT_EQUAL(string("1.2.3.4"), ifs["eth1"]["options"]["netmask"][(uint)0].get<string>() );
		CPPUNIT_ASSERT_EQUAL(string("1.1.1.1"), ifs["eth1"]["options"]["gateway"][(uint)0].get<string>() );

		CPPUNIT_ASSERT( ifs.contains("lo") );
		CPPUNIT_ASSERT( ifs.contains("eth0") );
		CPPUNIT_ASSERT( ifs.contains("eth1") );

		CPPUNIT_ASSERT( ifs["lo"]["auto"].get<bool>() );
		CPPUNIT_ASSERT( ifs["eth0"]["auto"].get<bool>() );
		CPPUNIT_ASSERT( ifs["eth1"]["auto"].get<bool>() );

	}
#endif
}

void TestNetworkConfig::TestDefaultRoute()
{
	//cout << "\n\nDefault route is: " << NetUtils::GetDefaultRoute() << "\n\n"  << endl;
	CPPUNIT_ASSERT( string("") != NetUtils::GetDefaultRoute() );
}

void TestNetworkConfig::TestDefaultDevice()
{
	//cout << "\n\nDefault device is: " << NetUtils::GetDefaultDevice() << "\n\n"  << endl;
	CPPUNIT_ASSERT( ""s != NetUtils::GetDefaultDevice() );
}

void TestNetworkConfig::TestNetmask()
{
	CPPUNIT_ASSERT_EQUAL( string(""), NetUtils::GetNetmask("NoInterface") );
	CPPUNIT_ASSERT( string("") != NetUtils::GetNetmask( sysinfo.NetworkDevice() ) );
}

void TestNetworkConfig::TestAddress()
{
	CPPUNIT_ASSERT_EQUAL( string(""), NetUtils::GetAddress("NoInterface") );
	CPPUNIT_ASSERT( string("") != NetUtils::GetAddress( sysinfo.NetworkDevice() ) );
}
