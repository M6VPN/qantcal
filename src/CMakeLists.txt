# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# src/CMakeLists.txt

add_library(qantcal_core
	calculators/antenna_calculator.cpp
	calculators/antenna_calculator.h
	calculators/coil_calculator.cpp
	calculators/coil_calculator.h
	calculators/coax_loss_calculator.cpp
	calculators/coax_loss_calculator.h
	calculators/ham_band.cpp
	calculators/ham_band.h
	calculators/lf_mf_antenna_calculator.cpp
	calculators/lf_mf_antenna_calculator.h
	calculators/lc_resonance_calculator.cpp
	calculators/lc_resonance_calculator.h
	calculators/loading_coil_calculator.cpp
	calculators/loading_coil_calculator.h
	calculators/radio_horizon_calculator.cpp
	calculators/radio_horizon_calculator.h
	calculators/rf_units.cpp
	calculators/rf_units.h
	calculators/swr_calculator.cpp
	calculators/swr_calculator.h
	calculators/trap_calculator.cpp
	calculators/trap_calculator.h
	calculators/yagi_calculator.cpp
	calculators/yagi_calculator.h
	guides/guide_document.cpp
	guides/guide_document.h
	guides/guide_export_options.cpp
	guides/guide_export_options.h
	guides/guide_renderer.cpp
	guides/guide_renderer.h
	guides/guide_section.cpp
	guides/guide_section.h
	project/antenna_project.cpp
	project/antenna_project.h
	project/project_file_io.cpp
	project/project_file_io.h
	project/project_serializer.cpp
	project/project_serializer.h
	reference/band_reference.cpp
	reference/band_reference.h
	reference/broadcast_band_reference.cpp
	reference/broadcast_band_reference.h
	reference/lf_mf_reference.cpp
	reference/lf_mf_reference.h
	reference/mode_reference.cpp
	reference/mode_reference.h
	reference/propagation_notes.cpp
	reference/propagation_notes.h
	reference/reach_estimator.cpp
	reference/reach_estimator.h
	settings/app_settings.cpp
	settings/app_settings.h
)

target_include_directories(qantcal_core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(qantcal_core
	PUBLIC
		Qt6::Core
		Qt6::Gui
		Qt6::PrintSupport
)

add_library(qantcal_ui
	design/antenna_design_item.cpp
	design/antenna_design_item.h
	design/antenna_design_scene.cpp
	design/antenna_design_scene.h
	design/antenna_design_view.cpp
	design/antenna_design_view.h
	design/antenna_measurement.cpp
	design/antenna_measurement.h
	design/design_commands.cpp
	design/design_commands.h
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
		Qt6::Widgets
)

qt_add_executable(qantcal
	main.cpp
)

target_link_libraries(qantcal
	PRIVATE
		qantcal_ui
)

install(
	TARGETS qantcal
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	BUNDLE DESTINATION .
)
