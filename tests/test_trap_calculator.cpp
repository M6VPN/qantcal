// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_trap_calculator.cpp

#include "calculators/trap_calculator.h"

#include <cassert>
#include <cmath>
#include <limits>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_invalid_inputs()
{
	qantcal::calculators::TrapCalculationInput input;

	assert(!qantcal::calculators::calculate_trap(input).ok);

	input.inductance_uh = 1.0;
	input.capacitance_pf = -100.0;
	assert(!qantcal::calculators::calculate_trap(input).ok);

	input.capacitance_pf = std::numeric_limits<double>::infinity();
	assert(!qantcal::calculators::calculate_trap(input).ok);

	input.mode = qantcal::calculators::TrapCalculationMode::InductanceFromFrequencyCapacitance;
	input.frequency_mhz = 0.0;
	input.capacitance_pf = 100.0;
	assert(!qantcal::calculators::calculate_trap(input).ok);
}

void
test_operating_reactance()
{
	qantcal::calculators::TrapCalculationInput input;

	input.inductance_uh = 1.0;
	input.capacitance_pf = 100.0;
	input.operating_frequency_mhz = 7.0;

	const qantcal::calculators::TrapCalculationResult result =
		qantcal::calculators::calculate_trap(input);

	assert(result.ok);
	assert(result.has_operating_reactance);
	assert(near_value(result.operating_inductive_reactance_ohms, 43.982, 0.010));
	assert(near_value(result.operating_capacitive_reactance_ohms, 227.364, 0.010));
}

void
test_reverse_capacitance()
{
	qantcal::calculators::TrapCalculationInput input;

	input.mode = qantcal::calculators::TrapCalculationMode::CapacitanceFromFrequencyInductance;
	input.frequency_mhz = 15.915494;
	input.inductance_uh = 1.0;

	const qantcal::calculators::TrapCalculationResult result =
		qantcal::calculators::calculate_trap(input);

	assert(result.ok);
	assert(near_value(result.capacitance_pf, 100.0, 0.010));
}

void
test_reverse_inductance()
{
	qantcal::calculators::TrapCalculationInput input;

	input.mode = qantcal::calculators::TrapCalculationMode::InductanceFromFrequencyCapacitance;
	input.frequency_mhz = 15.915494;
	input.capacitance_pf = 100.0;

	const qantcal::calculators::TrapCalculationResult result =
		qantcal::calculators::calculate_trap(input);

	assert(result.ok);
	assert(near_value(result.inductance_uh, 1.0, 0.010));
}

void
test_trap_resonance_sanity()
{
	qantcal::calculators::TrapCalculationInput input;

	input.inductance_uh = 1.0;
	input.capacitance_pf = 100.0;

	const qantcal::calculators::TrapCalculationResult result =
		qantcal::calculators::calculate_trap(input);

	assert(result.ok);
	assert(near_value(result.frequency_mhz, 15.915, 0.010));
	assert(near_value(result.reactance_ohms, 100.0, 0.100));
}

void
test_warning_near_resonance()
{
	qantcal::calculators::TrapCalculationInput input;

	input.inductance_uh = 1.0;
	input.capacitance_pf = 100.0;
	input.operating_frequency_mhz = 15.8;

	const qantcal::calculators::TrapCalculationResult result =
		qantcal::calculators::calculate_trap(input);

	assert(result.ok);
	assert(result.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("extremely close")));
}

}

int
main()
{
	test_invalid_inputs();
	test_operating_reactance();
	test_reverse_capacitance();
	test_reverse_inductance();
	test_trap_resonance_sanity();
	test_warning_near_resonance();

	return 0;
}
