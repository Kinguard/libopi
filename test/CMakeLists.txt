pkg_check_modules( CPPUNIT cppunit>=1.12.1 )

set( testapp_src
	test.cpp
	TestAuthServer.cpp
	TestBackupHelper.cpp
	TestCryptoHelper.cpp
	TestDiskHelper.cpp
	TestDnsHelper.cpp
	TestFetchmailConfig.cpp
	TestHttpClient.cpp
	TestJsonHelper.cpp
	TestMailConfig.cpp
	TestMailAliasFile.cpp
	TestNetworkConfig.cpp
	TestNotification.cpp
	TestResolverConfig.cpp
	TestSmtpClient.cpp
	TestSysInfo.cpp
	TestSysConfig.cpp
	)

include_directories(
	"${PROJECT_SOURCE_DIR}"
)

add_definitions( -std=c++11 )
add_executable( testapp ${testapp_src} )

target_link_libraries( testapp opi ${CPPUNIT_LDFLAGS} ${LIBUTILS_LDFLAGS} )

