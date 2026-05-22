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
