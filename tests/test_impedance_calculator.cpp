// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_impedance_calculator.cpp

#include "calculators/impedance_calculator.h"

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
test_capacitive_reactance_component()
{
	qantcal::calculators::ImpedanceCalculationInput input;

	input.frequency_mhz = 14.2;
	input.resistance_ohms = 50.0;
	input.reactance_ohms = -25.0;
	input.system_impedance_ohms = 50.0;

	const qantcal::calculators::ImpedanceCalculationResult result =
		qantcal::calculators::calculate_impedance(input);

	assert(result.ok);
	assert(result.has_reactive_component);
	assert(!result.reactance_is_inductive);
	assert(result.equivalent_capacitance_pf > 0.0);
	assert(result.equivalent_inductance_uh == 0.0);
}

void
test_complex_admittance_and_phase()
{
	qantcal::calculators::ImpedanceCalculationInput input;

	input.frequency_mhz = 14.2;
	input.resistance_ohms = 50.0;
	input.reactance_ohms = 25.0;
	input.system_impedance_ohms = 50.0;

	const qantcal::calculators::ImpedanceCalculationResult result =
		qantcal::calculators::calculate_impedance(input);

	assert(result.ok);
	assert(near_value(result.impedance_magnitude_ohms, 55.9017, 0.001));
	assert(near_value(result.phase_degrees, 26.5650, 0.001));
	assert(near_value(result.conductance_siemens, 0.0160, 0.0001));
	assert(near_value(result.susceptance_siemens, -0.0080, 0.0001));
}

void
test_inductive_reactance_component()
{
	qantcal::calculators::ImpedanceCalculationInput input;

	input.frequency_mhz = 14.2;
	input.resistance_ohms = 50.0;
	input.reactance_ohms = 25.0;
	input.system_impedance_ohms = 50.0;

	const qantcal::calculators::ImpedanceCalculationResult result =
		qantcal::calculators::calculate_impedance(input);

	assert(result.ok);
	assert(result.has_reactive_component);
	assert(result.reactance_is_inductive);
	assert(result.equivalent_inductance_uh > 0.0);
	assert(result.equivalent_capacitance_pf == 0.0);
}

void
test_invalid_inputs()
{
	qantcal::calculators::ImpedanceCalculationInput input;

	assert(!qantcal::calculators::calculate_impedance(input).ok);

	input.frequency_mhz = 14.2;
	input.resistance_ohms = -1.0;
	input.system_impedance_ohms = 50.0;
	assert(!qantcal::calculators::calculate_impedance(input).ok);

	input.resistance_ohms = 50.0;
	input.reactance_ohms = std::numeric_limits<double>::infinity();
	assert(!qantcal::calculators::calculate_impedance(input).ok);
}

void
test_perfect_match()
{
	qantcal::calculators::ImpedanceCalculationInput input;

	input.frequency_mhz = 14.2;
	input.resistance_ohms = 50.0;
	input.reactance_ohms = 0.0;
	input.system_impedance_ohms = 50.0;

	const qantcal::calculators::ImpedanceCalculationResult result =
		qantcal::calculators::calculate_impedance(input);

	assert(result.ok);
	assert(result.is_perfect_match);
	assert(result.reflection_coefficient_magnitude == 0.0);
	assert(result.swr == 1.0);
	assert(std::isinf(result.return_loss_db));
}

void
test_two_to_one_swr()
{
	qantcal::calculators::ImpedanceCalculationInput input;

	input.frequency_mhz = 14.2;
	input.resistance_ohms = 100.0;
	input.reactance_ohms = 0.0;
	input.system_impedance_ohms = 50.0;

	const qantcal::calculators::ImpedanceCalculationResult result =
		qantcal::calculators::calculate_impedance(input);

	assert(result.ok);
	assert(near_value(result.swr, 2.0, 0.001));
	assert(near_value(result.reflection_coefficient_magnitude, 1.0 / 3.0, 0.001));
}

}

int
main()
{
	test_capacitive_reactance_component();
	test_complex_admittance_and_phase();
	test_inductive_reactance_component();
	test_invalid_inputs();
	test_perfect_match();
	test_two_to_one_swr();

	return 0;
}
