// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_matching_network_calculator.cpp

#include "calculators/matching_network_calculator.h"

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
test_equal_resistance_needs_no_network()
{
	qantcal::calculators::MatchingNetworkInput input;

	input.frequency_mhz = 14.2;
	input.source_resistance_ohms = 50.0;
	input.load_resistance_ohms = 50.0;

	const qantcal::calculators::MatchingNetworkResult result =
		qantcal::calculators::calculate_matching_network(input);

	assert(result.ok);
	assert(result.no_network_needed);
	assert(result.q == 0.0);
	assert(result.note.contains(QStringLiteral("not needed")));
}

void
test_invalid_inputs()
{
	qantcal::calculators::MatchingNetworkInput input;

	assert(!qantcal::calculators::calculate_matching_network(input).ok);

	input.frequency_mhz = 14.2;
	input.source_resistance_ohms = 50.0;
	input.load_resistance_ohms = 0.0;
	assert(!qantcal::calculators::calculate_matching_network(input).ok);

	input.load_resistance_ohms = std::numeric_limits<double>::infinity();
	assert(!qantcal::calculators::calculate_matching_network(input).ok);
}

void
test_reversed_values_keep_same_magnitudes()
{
	qantcal::calculators::MatchingNetworkInput first_input;
	qantcal::calculators::MatchingNetworkInput second_input;

	first_input.frequency_mhz = 14.2;
	first_input.source_resistance_ohms = 50.0;
	first_input.load_resistance_ohms = 200.0;

	second_input.frequency_mhz = 14.2;
	second_input.source_resistance_ohms = 200.0;
	second_input.load_resistance_ohms = 50.0;

	const qantcal::calculators::MatchingNetworkResult first_result =
		qantcal::calculators::calculate_matching_network(first_input);
	const qantcal::calculators::MatchingNetworkResult second_result =
		qantcal::calculators::calculate_matching_network(second_input);

	assert(first_result.ok);
	assert(second_result.ok);
	assert(first_result.source_is_low_resistance_side);
	assert(!second_result.source_is_low_resistance_side);
	assert(near_value(first_result.series_reactance_ohms, second_result.series_reactance_ohms, 0.001));
	assert(near_value(first_result.shunt_reactance_ohms, second_result.shunt_reactance_ohms, 0.001));
}

void
test_resistive_l_network_values()
{
	qantcal::calculators::MatchingNetworkInput input;

	input.frequency_mhz = 14.2;
	input.source_resistance_ohms = 50.0;
	input.load_resistance_ohms = 200.0;

	const qantcal::calculators::MatchingNetworkResult result =
		qantcal::calculators::calculate_matching_network(input);

	assert(result.ok);
	assert(!result.no_network_needed);
	assert(near_value(result.q, std::sqrt(3.0), 0.001));
	assert(near_value(result.series_reactance_ohms, 86.6025, 0.001));
	assert(near_value(result.shunt_reactance_ohms, 115.4701, 0.001));
	assert(result.low_pass_series_inductance_uh > 0.0);
	assert(result.low_pass_shunt_capacitance_pf > 0.0);
	assert(result.high_pass_series_capacitance_pf > 0.0);
	assert(result.high_pass_shunt_inductance_uh > 0.0);
	assert(result.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("purely resistive")));
}

}

int
main()
{
	test_equal_resistance_needs_no_network();
	test_invalid_inputs();
	test_reversed_values_keep_same_magnitudes();
	test_resistive_l_network_values();

	return 0;
}
