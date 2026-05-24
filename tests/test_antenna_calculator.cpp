// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_antenna_calculator.cpp

#include "calculators/antenna_calculator.h"
#include "calculators/rf_units.h"

#include <cassert>
#include <cmath>
#include <string>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

double
expected_length(double frequency_mhz, double wave_ratio, double factor = qantcal::calculators::DEFAULT_WIRE_FACTOR)
{
	return qantcal::calculators::SPEED_OF_LIGHT_MPS / (frequency_mhz * 1.0e6) * wave_ratio * factor;
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

bool
has_warning(const qantcal::calculators::AntennaCalculationResult &result, const std::string &needle)
{
	for (const std::string &warning : result.warnings) {
		if (warning.find(needle) != std::string::npos)
			return true;
	}

	return false;
}

void
test_broadcast_49m_dipole()
{
	const qantcal::calculators::AntennaCalculationResult result =
		calculate(qantcal::calculators::AntennaType::HalfWaveDipole, 6.050);

	assert(result.ok);
	assert(near_value(result.total_length_m, 23.537, 0.010));
	assert(near_value(result.leg_length_m, 11.768, 0.010));
}

void
test_custom_shortening_factor_scales_length()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.frequency_mhz = 7.1;
	input.shortening_factor = 0.90;
	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(result.ok);
	assert(near_value(result.total_length_m, expected_length(7.1, 0.5, 0.90), 0.000001));
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
test_effectively_impossible_low_frequency_warns()
{
	const qantcal::calculators::AntennaCalculationResult result =
		calculate(qantcal::calculators::AntennaType::HalfWaveDipole, 0.001);

	assert(result.ok);
	assert(has_warning(result, "effectively impossible"));
	assert(has_warning(result, "LF/VLF"));
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
test_formula_reference_bands()
{
	const qantcal::calculators::AntennaCalculationResult top_band =
		calculate(qantcal::calculators::AntennaType::HalfWaveDipole, 1.9);
	const qantcal::calculators::AntennaCalculationResult forty =
		calculate(qantcal::calculators::AntennaType::HalfWaveDipole, 7.1);
	const qantcal::calculators::AntennaCalculationResult twenty_loop =
		calculate(qantcal::calculators::AntennaType::FullWaveLoop, 14.2);
	const qantcal::calculators::AntennaCalculationResult two_vertical =
		calculate(qantcal::calculators::AntennaType::QuarterWaveVertical, 144.3);
	const qantcal::calculators::AntennaCalculationResult seventy_cm =
		calculate(qantcal::calculators::AntennaType::HalfWaveDipole, 433.5);

	assert(top_band.ok);
	assert(forty.ok);
	assert(twenty_loop.ok);
	assert(two_vertical.ok);
	assert(seventy_cm.ok);
	assert(near_value(top_band.total_length_m, expected_length(1.9, 0.5), 0.000001));
	assert(near_value(forty.total_length_m, expected_length(7.1, 0.5), 0.000001));
	assert(near_value(twenty_loop.total_length_m, expected_length(14.2, 1.0), 0.000001));
	assert(near_value(two_vertical.radiator_length_m, expected_length(144.3, 0.25), 0.000001));
	assert(near_value(seventy_cm.total_length_m, expected_length(433.5, 0.5), 0.000001));
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
test_lf_vlf_vertical_warns()
{
	const qantcal::calculators::AntennaCalculationResult result =
		calculate(qantcal::calculators::AntennaType::QuarterWaveVertical, 0.05);

	assert(result.ok);
	assert(has_warning(result, "LF/VLF"));
	assert(has_warning(result, "effectively impossible"));
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
test_invalid_negative_converted_length()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.design_mode = qantcal::calculators::DesignMode::LengthToFrequency;
	input.length_m = qantcal::calculators::length_unit_to_metres(
		-2000.0,
		qantcal::calculators::LengthUnit::Centimetres
	);

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
test_microwave_wire_formula_warns()
{
	const qantcal::calculators::AntennaCalculationResult result =
		calculate(qantcal::calculators::AntennaType::HalfWaveDipole, 1300.0);

	assert(result.ok);
	assert(has_warning(result, "microwave"));
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
test_reverse_efhw_length_to_frequency()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::EndFedHalfWave;
	input.design_mode = qantcal::calculators::DesignMode::LengthToFrequency;
	input.length_m = expected_length(14.2, 0.5);

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(result.ok);
	assert(near_value(result.frequency_mhz, 14.2, 0.000001));
}

void
test_reverse_loop_length_to_frequency()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::FullWaveLoop;
	input.design_mode = qantcal::calculators::DesignMode::LengthToFrequency;
	input.length_m = expected_length(14.2, 1.0);

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(result.ok);
	assert(near_value(result.frequency_mhz, 14.2, 0.000001));
}

void
test_reverse_small_length_warns()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.design_mode = qantcal::calculators::DesignMode::LengthToFrequency;
	input.length_m = 0.002;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(result.ok);
	assert(has_warning(result, "construction tolerance"));
}

void
test_reverse_vertical_length_to_frequency()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::QuarterWaveVertical;
	input.design_mode = qantcal::calculators::DesignMode::LengthToFrequency;
	input.length_m = expected_length(14.2, 0.25);

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(result.ok);
	assert(near_value(result.frequency_mhz, 14.2, 0.000001));
}

void
test_reverse_centimetres_to_frequency()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.design_mode = qantcal::calculators::DesignMode::LengthToFrequency;
	input.length_m = qantcal::calculators::length_unit_to_metres(
		2005.7,
		qantcal::calculators::LengthUnit::Centimetres
	);

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	assert(result.ok);
	assert(near_value(result.frequency_mhz, 7.1, 0.005));
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
	test_broadcast_49m_dipole();
	test_custom_shortening_factor_scales_length();
	test_dipole_7_1_mhz();
	test_effectively_impossible_low_frequency_warns();
	test_efhw_equals_half_wave_starting_length();
	test_formula_reference_bands();
	test_full_wave_loop();
	test_lf_vlf_vertical_warns();
	test_invalid_factor_above_one();
	test_invalid_factor_below_minimum();
	test_invalid_negative_converted_length();
	test_invalid_negative_frequency();
	test_invalid_zero_length();
	test_microwave_wire_formula_warns();
	test_quarter_wave_14_2_mhz();
	test_reverse_centimetres_to_frequency();
	test_reverse_efhw_length_to_frequency();
	test_reverse_length_to_frequency();
	test_reverse_loop_length_to_frequency();
	test_reverse_small_length_warns();
	test_reverse_vertical_length_to_frequency();

	return 0;
}
