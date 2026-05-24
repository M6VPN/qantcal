# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(qantcal
	VERSION 0.1.0
	LANGUAGES CXX
)

set(QANTCAL_DISPLAY_VERSION "${PROJECT_VERSION}-alpha")

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
