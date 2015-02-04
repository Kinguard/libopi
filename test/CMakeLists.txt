pkg_check_modules( CPPUNIT cppunit>=1.12.1 )

set( testapp_src
	test.cpp
	TestBackupHelper.cpp
	TestMailConfig.cpp
	TestFetchmailConfig.cpp
	TestMailAliasFile.cpp
	TestNetworkConfig.cpp
	TestResolverConfig.cpp
	TestSmtpClient.cpp
	)

include_directories(
    "${PROJECT_SOURCE_DIR}"
)

add_definitions( -std=c++11 )
add_executable( testapp ${testapp_src} )

target_link_libraries( testapp opi ${CPPUNIT_LDFLAGS} ${LIBUTILS_LDFLAGS} )

