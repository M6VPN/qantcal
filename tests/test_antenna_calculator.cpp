// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_antenna_calculator.cpp

#include "calculators/antenna_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

qantcal::calculators::AntennaCalculationResult
calculate(qantcal::calculators::AntennaType antenna_type, double frequency_mhz)
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = antenna_type;
	input.design_mode = qantcal::calculators::DesignMode::FrequencyToLength;
	input.frequency_mhz = frequency_mhz;

	return qantcal::calculators::calculate_antenna(input);
}

void
test_dipole_7_1_mhz()
{
	const qantcal::calculators::AntennaCalculationResult result =
		calculate(qantcal::calculators::AntennaType::HalfWaveDipole, 7.1);

	assert(result.ok);
	assert(near_value(result.total_length_m, 20.057, 0.010));
	assert(near_value(result.leg_length_m, 10.028, 0.010));
	assert(near_value(result.total_length_ft, 65.806, 0.050));
}

void
test_efhw_equals_half_wave_starting_length()
{
	const qantcal::calculators::AntennaCalculationResult dipole =
		calculate(qantcal::calculators::AntennaType::HalfWaveDipole, 14.2);
	const qantcal::calculators::AntennaCalculationResult efhw =
		calculate(qantcal::calculators::AntennaType::EndFedHalfWave, 14.2);

	assert(dipole.ok);
	assert(efhw.ok);
	assert(near_value(efhw.total_length_m, dipole.total_length_m, 0.001));
}

void
test_full_wave_loop()
{
	const qantcal::calculators::AntennaCalculationResult result =
		calculate(qantcal::calculators::AntennaType::FullWaveLoop, 14.2);

	assert(result.ok);
	assert(near_value(result.total_length_m, 20.057, 0.010));
}

void
test_invalid_factor_above_one()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.frequency_mhz = 7.1;
	input.shortening_factor = 1.01;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(!result.ok);
	assert(!result.error.empty());
}

void
test_invalid_factor_below_minimum()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.frequency_mhz = 7.1;
	input.shortening_factor = 0.49;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(!result.ok);
	assert(!result.error.empty());
}

void
test_invalid_negative_frequency()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.design_mode = qantcal::calculators::DesignMode::FrequencyToLength;
	input.frequency_mhz = -7.1;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(!result.ok);
	assert(!result.error.empty());
}

void
test_invalid_zero_length()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.design_mode = qantcal::calculators::DesignMode::LengthToFrequency;
	input.length_m = 0.0;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(!result.ok);
	assert(!result.error.empty());
}

void
test_quarter_wave_14_2_mhz()
{
	const qantcal::calculators::AntennaCalculationResult result =
		calculate(qantcal::calculators::AntennaType::QuarterWaveVertical, 14.2);

	assert(result.ok);
	assert(near_value(result.radiator_length_m, 5.014, 0.010));
	assert(near_value(result.total_length_m, 5.014, 0.010));
}

void
test_reverse_length_to_frequency()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.design_mode = qantcal::calculators::DesignMode::LengthToFrequency;
	input.length_m = 20.057;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(result.ok);
	assert(near_value(result.frequency_mhz, 7.1, 0.005));
}

}

int
main()
{
	test_dipole_7_1_mhz();
	test_efhw_equals_half_wave_starting_length();
	test_full_wave_loop();
	test_invalid_factor_above_one();
	test_invalid_factor_below_minimum();
	test_invalid_negative_frequency();
	test_invalid_zero_length();
	test_quarter_wave_14_2_mhz();
	test_reverse_length_to_frequency();

	return 0;
}
