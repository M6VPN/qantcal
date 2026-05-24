// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/matching_network_calculator.cpp

#include "matching_network_calculator.h"

#include <algorithm>
#include <cmath>

namespace qantcal::calculators {

namespace {

constexpr double PI = 3.14159265358979323846;

bool
is_positive(double value)
{
	return std::isfinite(value) && value > 0.0;
}

MatchingNetworkResult
invalid_result(const QString &error_message)
{
	MatchingNetworkResult result;

	result.error_message = error_message;

	return result;
}

double
capacitance_pf_from_reactance(double frequency_hz, double reactance_ohms)
{
	return 1000000000000.0 / (2.0 * PI * frequency_hz * reactance_ohms);
}

double
inductance_uh_from_reactance(double frequency_hz, double reactance_ohms)
{
	return reactance_ohms * 1000000.0 / (2.0 * PI * frequency_hz);
}

void
add_warnings(MatchingNetworkResult &result)
{
	result.warnings << QStringLiteral("This helper assumes purely resistive source and load values. Real antenna feedpoints often include reactance.");
	result.warnings << QStringLiteral("Component voltage, current, Q, losses, stray capacitance, and layout are not modelled.");
	if (result.q > 5.0)
		result.warnings << QStringLiteral("High loaded Q can make the match narrow-band and component-sensitive.");
	if (result.low_pass_shunt_capacitance_pf < 1.0 || result.high_pass_series_capacitance_pf < 1.0)
		result.warnings << QStringLiteral("One capacitance value is very small; stray capacitance can dominate the design.");
}

}

MatchingNetworkResult
calculate_matching_network(const MatchingNetworkInput &input)
{
	if (!is_positive(input.frequency_mhz))
		return invalid_result(QStringLiteral("Frequency must be greater than zero."));
	if (!is_positive(input.source_resistance_ohms))
		return invalid_result(QStringLiteral("Source resistance must be greater than zero."));
	if (!is_positive(input.load_resistance_ohms))
		return invalid_result(QStringLiteral("Load resistance must be greater than zero."));

	MatchingNetworkResult result;

	result.ok = true;
	result.load_resistance_ohms = input.load_resistance_ohms;
	result.source_resistance_ohms = input.source_resistance_ohms;
	result.high_resistance_ohms = std::max(input.source_resistance_ohms, input.load_resistance_ohms);
	result.low_resistance_ohms = std::min(input.source_resistance_ohms, input.load_resistance_ohms);
	result.ratio = result.high_resistance_ohms / result.low_resistance_ohms;
	result.source_is_low_resistance_side = input.source_resistance_ohms <= input.load_resistance_ohms;
	result.low_side_label = result.source_is_low_resistance_side
		? QStringLiteral("source")
		: QStringLiteral("load");
	result.high_side_label = result.source_is_low_resistance_side
		? QStringLiteral("load")
		: QStringLiteral("source");

	if (input.source_resistance_ohms == input.load_resistance_ohms) {
		result.no_network_needed = true;
		result.note = QStringLiteral("Source and load resistances are equal, so a resistive L-network match is not needed.");
		return result;
	}

	const double frequency_hz = input.frequency_mhz * 1000000.0;

	result.q = std::sqrt(result.ratio - 1.0);
	result.series_reactance_ohms = result.q * result.low_resistance_ohms;
	result.shunt_reactance_ohms = result.high_resistance_ohms / result.q;
	result.low_pass_series_inductance_uh = inductance_uh_from_reactance(frequency_hz, result.series_reactance_ohms);
	result.low_pass_shunt_capacitance_pf = capacitance_pf_from_reactance(frequency_hz, result.shunt_reactance_ohms);
	result.high_pass_series_capacitance_pf = capacitance_pf_from_reactance(frequency_hz, result.series_reactance_ohms);
	result.high_pass_shunt_inductance_uh = inductance_uh_from_reactance(frequency_hz, result.shunt_reactance_ohms);
	result.note = QStringLiteral("First-pass resistive L-network helper. Put the series element on the low-resistance side and the shunt element across the high-resistance side.");
	add_warnings(result);

	return result;
}

}
