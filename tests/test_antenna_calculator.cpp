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

void
test_dipole_7_1_mhz()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.design_mode = qantcal::calculators::DesignMode::FrequencyToLength;
	input.frequency_mhz = 7.1;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(result.ok);
	assert(near_value(result.total_length_m, 20.057, 0.010));
	assert(near_value(result.leg_length_m, 10.028, 0.010));
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
test_quarter_wave_14_2_mhz()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::QuarterWaveVertical;
	input.design_mode = qantcal::calculators::DesignMode::FrequencyToLength;
	input.frequency_mhz = 14.2;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

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
	test_invalid_negative_frequency();
	test_quarter_wave_14_2_mhz();
	test_reverse_length_to_frequency();

	return 0;
}
