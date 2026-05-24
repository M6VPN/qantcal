// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_choke_calculator.cpp

#include "calculators/choke_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_impedance_scales_by_turns_squared()
{
	qantcal::calculators::ChokeCalculationInput input;

	input.frequency_mhz = 14.2;
	input.reference_impedance_ohms = 100.0;
	input.target_impedance_ohms = 1000.0;
	input.turns = 3.0;

	const qantcal::calculators::ChokeCalculationResult result =
		qantcal::calculators::calculate_choke(input);

	assert(result.ok);
	assert(near_value(result.estimated_impedance_ohms, 900.0, 0.001));
	assert(near_value(result.target_ratio, 0.9, 0.001));
}

void
test_invalid_inputs()
{
	qantcal::calculators::ChokeCalculationInput input;

	assert(!qantcal::calculators::calculate_choke(input).ok);

	input.frequency_mhz = 14.2;
	input.reference_impedance_ohms = 100.0;
	input.target_impedance_ohms = 1000.0;
	input.turns = 0.0;
	assert(!qantcal::calculators::calculate_choke(input).ok);

	input.turns = 3.0;
	input.reference_impedance_ohms = -1.0;
	assert(!qantcal::calculators::calculate_choke(input).ok);
}

void
test_mix_notes()
{
	qantcal::calculators::ChokeCalculationInput input;

	input.frequency_mhz = 7.1;
	input.mix = qantcal::calculators::ChokeCoreMix::Mix31;
	input.reference_impedance_ohms = 150.0;
	input.target_impedance_ohms = 1000.0;
	input.turns = 4.0;

	const qantcal::calculators::ChokeCalculationResult result =
		qantcal::calculators::calculate_choke(input);

	assert(result.ok);
	assert(result.mix_note.contains(QStringLiteral("Mix 31")));
	assert(qantcal::calculators::choke_mix_label(input.mix) == QStringLiteral("Mix 31"));
}

void
test_suggested_turns()
{
	qantcal::calculators::ChokeCalculationInput input;

	input.frequency_mhz = 14.2;
	input.reference_impedance_ohms = 100.0;
	input.target_impedance_ohms = 1000.0;
	input.turns = 4.0;

	const qantcal::calculators::ChokeCalculationResult result =
		qantcal::calculators::calculate_choke(input);

	assert(result.ok);
	assert(result.suggested_turns == 4);
	assert(near_value(result.estimated_impedance_ohms, 1600.0, 0.001));
}

void
test_warning_when_below_target()
{
	qantcal::calculators::ChokeCalculationInput input;

	input.frequency_mhz = 14.2;
	input.reference_impedance_ohms = 100.0;
	input.target_impedance_ohms = 1000.0;
	input.turns = 2.0;

	const qantcal::calculators::ChokeCalculationResult result =
		qantcal::calculators::calculate_choke(input);

	assert(result.ok);
	assert(result.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("below the target")));
}

}

int
main()
{
	test_impedance_scales_by_turns_squared();
	test_invalid_inputs();
	test_mix_notes();
	test_suggested_turns();
	test_warning_when_below_target();

	return 0;
}
