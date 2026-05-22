// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_lc_resonance_calculator.cpp

#include "calculators/lc_resonance_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_lc_resonance_sanity()
{
	qantcal::calculators::LcResonanceInput input;

	input.inductance_uh = 1.0;
	input.capacitance_pf = 100.0;

	const qantcal::calculators::LcResonanceResult result =
		qantcal::calculators::calculate_lc_resonance(input);

	assert(result.ok);
	assert(near_value(result.frequency_mhz, 15.915, 0.010));
}

void
test_reverse_capacitance()
{
	qantcal::calculators::LcResonanceInput input;

	input.mode = qantcal::calculators::LcCalculationMode::CapacitanceFromFrequencyInductance;
	input.frequency_mhz = 15.915494;
	input.inductance_uh = 1.0;

	const qantcal::calculators::LcResonanceResult result =
		qantcal::calculators::calculate_lc_resonance(input);

	assert(result.ok);
	assert(near_value(result.capacitance_pf, 100.0, 0.010));
}

void
test_reverse_inductance()
{
	qantcal::calculators::LcResonanceInput input;

	input.mode = qantcal::calculators::LcCalculationMode::InductanceFromFrequencyCapacitance;
	input.frequency_mhz = 15.915494;
	input.capacitance_pf = 100.0;

	const qantcal::calculators::LcResonanceResult result =
		qantcal::calculators::calculate_lc_resonance(input);

	assert(result.ok);
	assert(near_value(result.inductance_uh, 1.0, 0.010));
}

}

int
main()
{
	test_lc_resonance_sanity();
	test_reverse_capacitance();
	test_reverse_inductance();

	return 0;
}
