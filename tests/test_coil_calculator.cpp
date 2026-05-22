// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_coil_calculator.cpp

#include "calculators/coil_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_invalid_dimensions()
{
	qantcal::calculators::CoilCalculationInput input;

	input.diameter_m = 0.0;
	input.length_m = 0.1;
	input.turns = 10.0;

	const qantcal::calculators::CoilCalculationResult result =
		qantcal::calculators::calculate_air_core_solenoid(input);

	assert(!result.ok);
	assert(!result.error.empty());
}

void
test_wheeler_sanity()
{
	qantcal::calculators::CoilCalculationInput input;

	input.diameter_m = 0.0254;
	input.length_m = 0.0508;
	input.turns = 10.0;

	const qantcal::calculators::CoilCalculationResult result =
		qantcal::calculators::calculate_air_core_solenoid(input);

	assert(result.ok);
	assert(near_value(result.inductance_uh, 1.020, 0.010));
}

}

int
main()
{
	test_invalid_dimensions();
	test_wheeler_sanity();

	return 0;
}
