# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# tests/CMakeLists.txt

add_executable(test_antenna_calculator
	test_antenna_calculator.cpp
)

add_executable(test_antenna_measurement
	test_antenna_measurement.cpp
)

add_executable(test_band_reference
	test_band_reference.cpp
)

add_executable(test_broadcast_band_reference
	test_broadcast_band_reference.cpp
)

add_executable(test_coil_calculator
	test_coil_calculator.cpp
)

add_executable(test_ham_band
	test_ham_band.cpp
)

add_executable(test_guide_document
	test_guide_document.cpp
)

add_executable(test_guide_renderer
	test_guide_renderer.cpp
)

add_executable(test_lc_resonance_calculator
	test_lc_resonance_calculator.cpp
)

add_executable(test_mode_reference
	test_mode_reference.cpp
)

add_executable(test_propagation_notes
	test_propagation_notes.cpp
)

add_executable(test_radio_horizon_calculator
	test_radio_horizon_calculator.cpp
)

add_executable(test_reach_estimator
	test_reach_estimator.cpp
)

add_executable(test_project_file_io
	test_project_file_io.cpp
)

add_executable(test_project_serializer
	test_project_serializer.cpp
)

add_executable(test_rf_units
	test_rf_units.cpp
)

add_executable(test_swr_calculator
	test_swr_calculator.cpp
)

add_executable(test_yagi_calculator
	test_yagi_calculator.cpp
)

target_link_libraries(test_antenna_calculator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_antenna_measurement
	PRIVATE
		qantcal_ui
)

target_link_libraries(test_band_reference
	PRIVATE
		qantcal_core
)

target_link_libraries(test_broadcast_band_reference
	PRIVATE
		qantcal_core
)

target_link_libraries(test_coil_calculator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_ham_band
	PRIVATE
		qantcal_core
)

target_link_libraries(test_guide_document
	PRIVATE
		qantcal_core
)

target_link_libraries(test_guide_renderer
	PRIVATE
		qantcal_core
)

target_link_libraries(test_lc_resonance_calculator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_mode_reference
	PRIVATE
		qantcal_core
)

target_link_libraries(test_propagation_notes
	PRIVATE
		qantcal_core
)

target_link_libraries(test_radio_horizon_calculator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_reach_estimator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_project_file_io
	PRIVATE
		qantcal_core
)

target_link_libraries(test_project_serializer
	PRIVATE
		qantcal_core
)

target_link_libraries(test_rf_units
	PRIVATE
		qantcal_core
)

target_link_libraries(test_swr_calculator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_yagi_calculator
	PRIVATE
		qantcal_core
)

add_test(
	NAME test_antenna_calculator
	COMMAND test_antenna_calculator
)

add_test(
	NAME test_antenna_measurement
	COMMAND test_antenna_measurement
)

add_test(
	NAME test_band_reference
	COMMAND test_band_reference
)

add_test(
	NAME test_broadcast_band_reference
	COMMAND test_broadcast_band_reference
)

add_test(
	NAME test_coil_calculator
	COMMAND test_coil_calculator
)

add_test(
	NAME test_ham_band
	COMMAND test_ham_band
)

add_test(
	NAME test_guide_document
	COMMAND test_guide_document
)

add_test(
	NAME test_guide_renderer
	COMMAND test_guide_renderer
)

add_test(
	NAME test_lc_resonance_calculator
	COMMAND test_lc_resonance_calculator
)

add_test(
	NAME test_mode_reference
	COMMAND test_mode_reference
)

add_test(
	NAME test_propagation_notes
	COMMAND test_propagation_notes
)

add_test(
	NAME test_radio_horizon_calculator
	COMMAND test_radio_horizon_calculator
)

add_test(
	NAME test_reach_estimator
	COMMAND test_reach_estimator
)

add_test(
	NAME test_project_file_io
	COMMAND test_project_file_io
)

add_test(
	NAME test_project_serializer
	COMMAND test_project_serializer
)

add_test(
	NAME test_rf_units
	COMMAND test_rf_units
)

add_test(
	NAME test_swr_calculator
	COMMAND test_swr_calculator
)

add_test(
	NAME test_yagi_calculator
	COMMAND test_yagi_calculator
)

set_tests_properties(
	test_antenna_calculator
	test_antenna_measurement
	test_band_reference
	test_broadcast_band_reference
	test_coil_calculator
	test_ham_band
	test_guide_document
	test_guide_renderer
	test_lc_resonance_calculator
	test_mode_reference
	test_propagation_notes
	test_radio_horizon_calculator
	test_reach_estimator
	test_project_file_io
	test_project_serializer
	test_rf_units
	test_swr_calculator
	test_yagi_calculator
	PROPERTIES
		ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
