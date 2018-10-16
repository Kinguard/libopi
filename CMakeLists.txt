project( opi )

cmake_minimum_required(VERSION 2.6)

include( FindPkgConfig )
pkg_check_modules ( LIBUTILS REQUIRED libutils>=1.5.1 )
pkg_check_modules ( LIBUDEV REQUIRED libudev )
pkg_check_modules ( LIBCRYPTSETUP REQUIRED libcryptsetup )
pkg_check_modules ( LIBPARTED REQUIRED libparted>=2.3 )
pkg_check_modules ( LIBCURL REQUIRED libcurl )
pkg_check_modules ( LIBCRYPTO++ REQUIRED libcrypto++>=5.6.1 )
pkg_check_modules ( CPPUNIT REQUIRED cppunit>=1.12.1)
pkg_check_modules ( LIBJSONCPP REQUIRED jsoncpp>=1.0 )
pkg_check_modules ( LIBSSL REQUIRED libssl )
pkg_check_modules ( BLKID REQUIRED blkid>=2.20.0 )
pkg_check_modules ( LVM2APP REQUIRED lvm2app )


set (VERSION_MAJOR 1)
set (VERSION_MINOR 6)
set (VERSION_PATCH 37)
set (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")
set (VERSION_FULL "${VERSION}.${VERSION_PATCH}")
set (APP_NAME "lib${PROJECT_NAME}")

configure_file(
	"${PROJECT_SOURCE_DIR}/Config.h.in"
	"${PROJECT_BINARY_DIR}/Config.h"
)

configure_file(
	"${PROJECT_SOURCE_DIR}/lib${PROJECT_NAME}.pc.in"
	"${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.pc"
	@ONLY
)

include_directories(
	"${PROJECT_BINARY_DIR}"
	"${LIBJSONCPP_INCLUDE_DIRS}"
	"${BLKID_INCLUDE_DIRS}"
	)

set( headers
	AuthServer.h
	BackupHelper.h
	BaseManager.h
	CryptoHelper.h
	DiskHelper.h
	DnsHelper.h
	DnsServer.h
	FetchmailConfig.h
	HttpClient.h
	IdentityManager.h
	LedControl.h
	Luks.h
	LVM.h
	MailConfig.h
	NetworkConfig.h
	Notification.h
	Secop.h
	ServiceHelper.h
	SmtpConfig.h
	SysConfig.h
	SysInfo.h
	ExtCert.h
	"${PROJECT_BINARY_DIR}/Config.h"
	)

set( src
	AuthServer.cpp
	BackupHelper.cpp
	BaseManager.cpp
	CryptoHelper.cpp
	DiskHelper.cpp
	DnsHelper.cpp
	DnsServer.cpp
	FetchmailConfig.cpp
	HttpClient.cpp
	IdentityManager.cpp
	LedControl.cpp
	Luks.cpp
	LVM.cpp
	MailConfig.cpp
	NetworkConfig.cpp
	Notification.cpp
	Secop.cpp
	ServiceHelper.cpp
	SmtpConfig.cpp
	SysConfig.cpp
	SysInfo.cpp
	ExtCert.cpp
	)

add_definitions( -std=c++11 -Wall )

add_library( ${PROJECT_NAME}_static STATIC ${src} )
add_library( ${PROJECT_NAME} SHARED ${src} )

target_link_libraries(  ${PROJECT_NAME}
	-ljsoncpp
	${LIBSSL_LDFLAGS}
	${LIBJSONCPP_LDFLAGS}
	${LIBUTILS_LDFLAGS}
	${LIBUDEV_LDFLAGS}
	${LIBCRYPTSETUP_LDFLAGS}
	${LIBPARTED_LDFLAGS}
	${LIBCURL_LDFLAGS}
	${LIBCRYPTO++_LDFLAGS}
	${BLKID_LDFLAGS}
	${LVM2APP_LDFLAGS}
	)

set_target_properties( ${PROJECT_NAME} PROPERTIES
		VERSION ${VERSION_FULL}
		SOVERSION ${VERSION_MAJOR}
	)

set_target_properties( ${PROJECT_NAME}_static PROPERTIES
		OUTPUT_NAME ${PROJECT_NAME}
	)

add_subdirectory( test )

enable_testing()
add_test( NAME TestLibOpi COMMAND testapp )

install( FILES ${headers} DESTINATION include/lib${PROJECT_NAME} )
install(FILES "${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.pc" DESTINATION lib/pkgconfig)
install(FILES "${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.pc" DESTINATION share/pkgconfig)
install( TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static DESTINATION lib )
