// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_rf_units.cpp

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

void
test_centimetres()
{
	assert(near_value(qantcal::calculators::metres_to_centimetres(1.25), 125.0, 0.001));
}

void
test_feet_inches_format()
{
	const std::string formatted = qantcal::calculators::format_length(
		qantcal::calculators::feet_to_metres(33.5),
		qantcal::calculators::LengthUnit::FeetInches
	);

	assert(formatted == "33 ft 6.0 in");
}

void
test_feet_to_metres()
{
	assert(near_value(qantcal::calculators::feet_to_metres(1.0), 0.3048, 0.000001));
}

void
test_inches_to_metres()
{
	assert(near_value(qantcal::calculators::inches_to_metres(1.0), 0.0254, 0.000001));
}

void
test_metres_to_feet()
{
	assert(near_value(qantcal::calculators::metres_to_feet(1.0), 3.280839895, 0.000001));
}

void
test_millimetres()
{
	assert(near_value(qantcal::calculators::metres_to_millimetres(1.25), 1250.0, 0.001));
}

void
test_parse_feet_inches_colon()
{
	double feet = 0.0;

	assert(qantcal::calculators::parse_feet_inches("33:6", feet));
	assert(near_value(feet, 33.5, 0.000001));
}

void
test_parse_feet_inches_decimal()
{
	double feet = 0.0;

	assert(qantcal::calculators::parse_feet_inches("33.5", feet));
	assert(near_value(feet, 33.5, 0.000001));
}

void
test_parse_feet_inches_invalid()
{
	double feet = 0.0;

	assert(!qantcal::calculators::parse_feet_inches("", feet));
	assert(!qantcal::calculators::parse_feet_inches("33 metres", feet));
	assert(!qantcal::calculators::parse_feet_inches("33 ft six in", feet));
	assert(!qantcal::calculators::parse_feet_inches("-33 ft 6 in", feet));
}

void
test_parse_feet_inches_symbols()
{
	double feet = 0.0;

	assert(qantcal::calculators::parse_feet_inches("33' 6\"", feet));
	assert(near_value(feet, 33.5, 0.000001));
	assert(qantcal::calculators::parse_feet_inches("33'6\"", feet));
	assert(near_value(feet, 33.5, 0.000001));
}

void
test_parse_feet_inches_words()
{
	double feet = 0.0;

	assert(qantcal::calculators::parse_feet_inches("33 ft 6 in", feet));
	assert(near_value(feet, 33.5, 0.000001));
	assert(qantcal::calculators::parse_feet_inches("33 feet 6 inches", feet));
	assert(near_value(feet, 33.5, 0.000001));
}

void
test_selected_unit_formatting_for_dipole()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	input.frequency_mhz = 7.1;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);
	const std::string formatted = qantcal::calculators::format_length(
		result.total_length_m,
		qantcal::calculators::LengthUnit::Centimetres
	);

	assert(result.ok);
	assert(formatted == "2005.65 cm");
}

}

int
main()
{
	test_centimetres();
	test_feet_inches_format();
	test_feet_to_metres();
	test_inches_to_metres();
	test_metres_to_feet();
	test_millimetres();
	test_parse_feet_inches_colon();
	test_parse_feet_inches_decimal();
	test_parse_feet_inches_invalid();
	test_parse_feet_inches_symbols();
	test_parse_feet_inches_words();
	test_selected_unit_formatting_for_dipole();

	return 0;
}
