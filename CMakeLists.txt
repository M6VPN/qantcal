# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(qantcal
	VERSION 0.1.0
	LANGUAGES CXX
)

set(QANTCAL_PRERELEASE "alpha.1")
set(QANTCAL_DISPLAY_VERSION "${PROJECT_VERSION}-${QANTCAL_PRERELEASE}")
set(QANTCAL_DEBIAN_VERSION "${PROJECT_VERSION}~${QANTCAL_PRERELEASE}")
set(QANTCAL_PACKAGE_RELEASE "1")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network PrintSupport)
find_package(Qt6 COMPONENTS LinguistTools QUIET)
qt_standard_project_setup()

set(QANTCAL_LRELEASE_COMMAND)
if(TARGET Qt6::lrelease)
	set(QANTCAL_LRELEASE_COMMAND Qt6::lrelease)
elseif(DEFINED Qt6_LRELEASE_EXECUTABLE AND EXISTS "${Qt6_LRELEASE_EXECUTABLE}")
	set(QANTCAL_LRELEASE_COMMAND "${Qt6_LRELEASE_EXECUTABLE}")
else()
	find_program(LRELEASE_EXECUTABLE
		NAMES lrelease lrelease-qt6
		PATHS
			/usr/lib/qt6/bin
			/usr/local/lib/qt6/bin
			/opt/homebrew/opt/qt/bin
		DOC "Qt lrelease executable"
	)
	if(LRELEASE_EXECUTABLE)
		set(QANTCAL_LRELEASE_COMMAND "${LRELEASE_EXECUTABLE}")
	endif()
endif()

if(NOT QANTCAL_LRELEASE_COMMAND)
	message(FATAL_ERROR "Qt lrelease was not found; install Qt Linguist tools before building qantcal.")
endif()

set(QANTCAL_TRANSLATION_SOURCES
	translations/qantcal_ru.ts
)

foreach(translation_source IN LISTS QANTCAL_TRANSLATION_SOURCES)
	get_filename_component(translation_name "${translation_source}" NAME_WE)
	set(translation_output "${CMAKE_CURRENT_BINARY_DIR}/translations/${translation_name}.qm")
	add_custom_command(
		OUTPUT "${translation_output}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/translations"
		COMMAND ${QANTCAL_LRELEASE_COMMAND} "${CMAKE_CURRENT_SOURCE_DIR}/${translation_source}" -qm "${translation_output}"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${translation_source}"
		COMMENT "Compiling ${translation_source}"
		VERBATIM
	)
	list(APPEND QANTCAL_TRANSLATION_OUTPUTS "${translation_output}")
endforeach()

add_custom_target(qantcal_translations ALL
	DEPENDS ${QANTCAL_TRANSLATION_OUTPUTS}
)

install(
	FILES ${QANTCAL_TRANSLATION_OUTPUTS}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/qantcal/translations
)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

install(
	FILES
		BUILDING.md
		LICENSE
		README.md
	DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

install(
	DIRECTORY docs/
	DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs
	FILES_MATCHING
		PATTERN "*.md"
)

install(
	FILES packaging/linux/qantcal.desktop
	DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(
	FILES packaging/linux/qantcal.svg
	DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

install(
	FILES packaging/linux/io.github.m6vpn.qantcal.metainfo.xml
	DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)

set(CPACK_PACKAGE_NAME "qantcal")
set(CPACK_PACKAGE_VENDOR "M6VPN")
set(CPACK_PACKAGE_CONTACT "M6VPN <M6VPN@tuta.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Qt6 amateur radio antenna calculator and build guide generator")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/M6VPN/qantcal")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "qantcal")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_NAME "qantcal")
set(CPACK_DEBIAN_PACKAGE_VERSION "${QANTCAL_DEBIAN_VERSION}")
set(CPACK_DEBIAN_PACKAGE_RELEASE "${QANTCAL_PACKAGE_RELEASE}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "M6VPN <M6VPN@tuta.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
set(CPACK_RPM_PACKAGE_NAME "qantcal")
set(CPACK_RPM_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_RPM_PACKAGE_RELEASE "0.${QANTCAL_PRERELEASE}.${QANTCAL_PACKAGE_RELEASE}")
set(CPACK_RPM_PACKAGE_LICENSE "ISC")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Engineering")
set(CPACK_RPM_PACKAGE_VENDOR "M6VPN")
set(CPACK_RPM_PACKAGE_URL "https://github.com/M6VPN/qantcal")
set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)

include(CPack)
