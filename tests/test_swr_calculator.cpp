// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_swr_calculator.cpp

#include "calculators/swr_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_invalid_swr()
{
	qantcal::calculators::SwrCalculationInput input;

	input.forward_power_w = 100.0;
	input.swr = 0.9;

	const qantcal::calculators::SwrCalculationResult result =
		qantcal::calculators::calculate_swr(input);

	assert(!result.ok);
	assert(!result.error.empty());
}

void
test_perfect_swr()
{
	qantcal::calculators::SwrCalculationInput input;

	input.forward_power_w = 100.0;
	input.swr = 1.0;

	const qantcal::calculators::SwrCalculationResult result =
		qantcal::calculators::calculate_swr(input);

	assert(result.ok);
	assert(result.reflected_power_w == 0.0);
	assert(result.reflected_percent == 0.0);
	assert(result.delivered_power_w == 100.0);
}

void
test_two_to_one_swr()
{
	qantcal::calculators::SwrCalculationInput input;

	input.forward_power_w = 100.0;
	input.swr = 2.0;

	const qantcal::calculators::SwrCalculationResult result =
		qantcal::calculators::calculate_swr(input);

	assert(result.ok);
	assert(near_value(result.reflection_coefficient, 0.333, 0.001));
	assert(near_value(result.reflected_percent, 11.111, 0.010));
	assert(near_value(result.reflected_power_w, 11.111, 0.010));
}

}

int
main()
{
	test_invalid_swr();
	test_perfect_swr();
	test_two_to_one_swr();

	return 0;
}
