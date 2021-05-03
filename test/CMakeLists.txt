pkg_check_modules( CPPUNIT cppunit>=1.12.1 )

set( testapp_src
	test.cpp
	TestAuthServer.cpp
	TestBackupHelper.cpp
	TestCryptoHelper.cpp
	TestDiskHelper.cpp
	TestDnsHelper.cpp
	TestFetchmailConfig.cpp
	TestHostsConfig.cpp
	TestHttpClient.cpp
	TestJsonHelper.cpp
	TestMailConfig.cpp
	TestMailAliasFile.cpp
	TestNetworkConfig.cpp
	TestNotification.cpp
	TestRaspbianNetworkConfig.cpp
	TestResolverConfig.cpp
	TestSmtpClient.cpp
	TestSysInfo.cpp
	TestSysConfig.cpp
	)

configure_file("dhcpcd.conf" "dhcpcd.conf" COPYONLY)

include_directories(
	"${PROJECT_SOURCE_DIR}"
)

add_definitions( -Wall )
add_executable( testapp ${testapp_src} )

target_link_libraries( testapp opi ${CPPUNIT_LDFLAGS} ${LIBUTILS_LDFLAGS} )

