# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# tests/CMakeLists.txt

add_executable(test_antenna_calculator
	test_antenna_calculator.cpp
)

add_executable(test_ham_band
	test_ham_band.cpp
)

target_link_libraries(test_antenna_calculator
	PRIVATE
		qantcal_core
)

target_link_libraries(test_ham_band
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
