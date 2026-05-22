# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# tests/CMakeLists.txt

add_executable(test_antenna_calculator
	test_antenna_calculator.cpp
)

add_executable(test_ham_band
	test_ham_band.cpp
)

add_executable(test_rf_units
	test_rf_units.cpp
)

target_link_libraries(test_antenna_calculator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_ham_band
	PRIVATE
		qantcal_core
)

target_link_libraries(test_rf_units
	PRIVATE
		qantcal_core
)

add_test(
	NAME test_antenna_calculator
	COMMAND test_antenna_calculator
)

add_test(
	NAME test_ham_band
	COMMAND test_ham_band
)

add_test(
	NAME test_rf_units
	COMMAND test_rf_units
)
