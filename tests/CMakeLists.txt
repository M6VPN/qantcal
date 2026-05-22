# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# tests/CMakeLists.txt

add_executable(test_antenna_calculator
	test_antenna_calculator.cpp
)

add_executable(test_coil_calculator
	test_coil_calculator.cpp
)

add_executable(test_ham_band
	test_ham_band.cpp
)

add_executable(test_lc_resonance_calculator
	test_lc_resonance_calculator.cpp
)

add_executable(test_radio_horizon_calculator
	test_radio_horizon_calculator.cpp
)

add_executable(test_rf_units
	test_rf_units.cpp
)

add_executable(test_swr_calculator
	test_swr_calculator.cpp
)

target_link_libraries(test_antenna_calculator
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

target_link_libraries(test_lc_resonance_calculator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_radio_horizon_calculator
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

add_test(
	NAME test_antenna_calculator
	COMMAND test_antenna_calculator
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
	NAME test_lc_resonance_calculator
	COMMAND test_lc_resonance_calculator
)

add_test(
	NAME test_radio_horizon_calculator
	COMMAND test_radio_horizon_calculator
)

add_test(
	NAME test_rf_units
	COMMAND test_rf_units
)

add_test(
	NAME test_swr_calculator
	COMMAND test_swr_calculator
)
