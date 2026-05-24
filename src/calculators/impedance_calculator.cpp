// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/impedance_calculator.cpp

#include "impedance_calculator.h"

#include <cmath>
#include <limits>

namespace qantcal::calculators {

namespace {

constexpr double PI = 3.14159265358979323846;

bool
is_finite(double value)
{
	return std::isfinite(value);
}

bool
is_positive(double value)
{
	return std::isfinite(value) && value > 0.0;
}

ImpedanceCalculationResult
invalid_result(const QString &error_message)
{
	ImpedanceCalculationResult result;

	result.error_message = error_message;

	return result;
}

void
add_warnings(ImpedanceCalculationResult &result)
{
	result.warnings << QStringLiteral("This helper analyses one measured impedance value. It does not design a tuner or matching network automatically.");
	if (result.swr > 10.0 || result.is_total_reflection)
		result.warnings << QStringLiteral("Very high SWR can stress transmitters, feedlines, tuners, and matching components.");
	if (result.has_reactive_component)
		result.warnings << QStringLiteral("Equivalent reactance values are single-frequency references only.");
}

}

ImpedanceCalculationResult
calculate_impedance(const ImpedanceCalculationInput &input)
{
	if (!is_positive(input.frequency_mhz))
		return invalid_result(QStringLiteral("Frequency must be greater than zero."));
	if (!is_positive(input.system_impedance_ohms))
		return invalid_result(QStringLiteral("System impedance must be greater than zero."));
	if (!is_finite(input.resistance_ohms) || input.resistance_ohms < 0.0)
		return invalid_result(QStringLiteral("Resistance must be zero or greater."));
	if (!is_finite(input.reactance_ohms))
		return invalid_result(QStringLiteral("Reactance must be finite."));

	const double frequency_hz = input.frequency_mhz * 1000000.0;
	const double impedance_denominator = input.resistance_ohms * input.resistance_ohms + input.reactance_ohms * input.reactance_ohms;
	const double gamma_real_numerator = input.resistance_ohms * input.resistance_ohms + input.reactance_ohms * input.reactance_ohms - input.system_impedance_ohms * input.system_impedance_ohms;
	const double gamma_imag_numerator = 2.0 * input.system_impedance_ohms * input.reactance_ohms;
	const double gamma_denominator = (input.resistance_ohms + input.system_impedance_ohms) * (input.resistance_ohms + input.system_impedance_ohms) + input.reactance_ohms * input.reactance_ohms;
	ImpedanceCalculationResult result;

	result.ok = true;
	result.impedance_magnitude_ohms = std::sqrt(impedance_denominator);
	result.phase_degrees = std::atan2(input.reactance_ohms, input.resistance_ohms) * 180.0 / PI;
	if (impedance_denominator > 0.0) {
		result.conductance_siemens = input.resistance_ohms / impedance_denominator;
		result.susceptance_siemens = -input.reactance_ohms / impedance_denominator;
		result.admittance_magnitude_siemens = std::sqrt(result.conductance_siemens * result.conductance_siemens + result.susceptance_siemens * result.susceptance_siemens);
	}
	result.reflection_coefficient_magnitude = std::sqrt(gamma_real_numerator * gamma_real_numerator + gamma_imag_numerator * gamma_imag_numerator) / gamma_denominator;
	result.is_perfect_match = result.reflection_coefficient_magnitude <= 0.000000001;
	result.is_total_reflection = result.reflection_coefficient_magnitude >= 0.999999999;
	if (result.is_perfect_match) {
		result.swr = 1.0;
		result.return_loss_db = std::numeric_limits<double>::infinity();
		result.mismatch_loss_db = 0.0;
	} else if (result.is_total_reflection) {
		result.swr = std::numeric_limits<double>::infinity();
		result.return_loss_db = -20.0 * std::log10(result.reflection_coefficient_magnitude);
		result.mismatch_loss_db = std::numeric_limits<double>::infinity();
	} else {
		result.swr = (1.0 + result.reflection_coefficient_magnitude) / (1.0 - result.reflection_coefficient_magnitude);
		result.return_loss_db = -20.0 * std::log10(result.reflection_coefficient_magnitude);
		result.mismatch_loss_db = -10.0 * std::log10(1.0 - result.reflection_coefficient_magnitude * result.reflection_coefficient_magnitude);
	}
	result.has_reactive_component = std::fabs(input.reactance_ohms) > 0.000000001;
	result.reactance_is_inductive = input.reactance_ohms > 0.0;
	if (result.has_reactive_component && result.reactance_is_inductive)
		result.equivalent_inductance_uh = input.reactance_ohms * 1000000.0 / (2.0 * PI * frequency_hz);
	else if (result.has_reactive_component)
		result.equivalent_capacitance_pf = 1000000000000.0 / (2.0 * PI * frequency_hz * std::fabs(input.reactance_ohms));
	result.note = QStringLiteral("Complex impedance helper for measured or entered impedance values. Matching design, tuner loss, component stress, and frequency sweeps are not modelled.");
	add_warnings(result);

	return result;
}

}
