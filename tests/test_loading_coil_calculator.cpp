// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_loading_coil_calculator.cpp

#include "calculators/loading_coil_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_lc_loading_inductance()
{
	qantcal::calculators::LoadingCoilInput input;

	input.frequency_mhz = 0.475;
	input.physical_vertical_height_metres = 10.0;
	input.has_estimated_capacitance = true;
	input.estimated_capacitance_pf = 200.0;
	const qantcal::calculators::LoadingCoilResult result =
		qantcal::calculators::calculate_loading_coil(input);

	assert(result.ok);
	assert(result.has_inductance);
	assert(near(result.inductance_uh, 561.5, 2.0));
}

void
test_missing_capacitance_does_not_crash()
{
	qantcal::calculators::LoadingCoilInput input;

	input.frequency_mhz = 0.475;
	input.physical_vertical_height_metres = 10.0;
	const qantcal::calculators::LoadingCoilResult result =
		qantcal::calculators::calculate_loading_coil(input);

	assert(result.ok);
	assert(!result.has_inductance);
	assert(!result.warnings.isEmpty());
}

}

int
main()
{
	test_lc_loading_inductance();
	test_missing_capacitance_does_not_crash();

	return 0;
}
