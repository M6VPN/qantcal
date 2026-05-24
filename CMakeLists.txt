# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(qantcal
	VERSION 0.1.0
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
qt_standard_project_setup()

find_program(LRELEASE_EXECUTABLE NAMES lrelease lrelease-qt6)

set(QANTCAL_TRANSLATION_SOURCES
	translations/qantcal_ru.ts
)

if(LRELEASE_EXECUTABLE)
	foreach(translation_source IN LISTS QANTCAL_TRANSLATION_SOURCES)
		get_filename_component(translation_name "${translation_source}" NAME_WE)
		set(translation_output "${CMAKE_CURRENT_BINARY_DIR}/translations/${translation_name}.qm")
		add_custom_command(
			OUTPUT "${translation_output}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/translations"
			COMMAND ${LRELEASE_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/${translation_source}" -qm "${translation_output}"
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
else()
	message(WARNING "lrelease was not found; translation .qm files will not be built.")
endif()

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
