# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# src/CMakeLists.txt

add_library(qantcal_core
	calculators/antenna_calculator.cpp
	calculators/antenna_calculator.h
	calculators/rf_units.cpp
	calculators/rf_units.h
)

target_include_directories(qantcal_core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(qantcal_ui
	design/antenna_design_scene.cpp
	design/antenna_design_scene.h
	main_window.cpp
	main_window.h
)

target_include_directories(qantcal_ui
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(qantcal_ui
	PUBLIC
		qantcal_core
		Qt6::Core
		Qt6::Gui
		Qt6::Widgets
		Qt6::PrintSupport
)

qt_add_executable(qantcal
	main.cpp
)

target_link_libraries(qantcal
	PRIVATE
		qantcal_ui
)
