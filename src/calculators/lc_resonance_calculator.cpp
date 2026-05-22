// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/lc_resonance_calculator.cpp

#include "lc_resonance_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

constexpr double PI = 3.14159265358979323846;

LcResonanceResult
invalid_result(const std::string &error)
{
	LcResonanceResult result;

	result.error = error;

	return result;
}

bool
is_positive(double value)
{
	return std::isfinite(value) && value > 0.0;
}

void
set_frequency_fields(LcResonanceResult &result, double frequency_hz)
{
	result.frequency_hz = frequency_hz;
	result.frequency_khz = frequency_hz / 1000.0;
	result.frequency_mhz = frequency_hz / 1000000.0;
}

}

LcResonanceResult
calculate_lc_resonance(const LcResonanceInput &input)
{
	LcResonanceResult result;

	switch (input.mode) {
	case LcCalculationMode::FrequencyFromLC: {
		if (!is_positive(input.inductance_uh))
			return invalid_result("Inductance must be greater than zero.");
		if (!is_positive(input.capacitance_pf))
			return invalid_result("Capacitance must be greater than zero.");

		const double inductance_h = input.inductance_uh * 0.000001;
		const double capacitance_f = input.capacitance_pf * 0.000000000001;
		const double frequency_hz = 1.0 / (2.0 * PI * std::sqrt(inductance_h * capacitance_f));

		result.ok = true;
		result.inductance_uh = input.inductance_uh;
		result.capacitance_pf = input.capacitance_pf;
		set_frequency_fields(result, frequency_hz);
		return result;
	}
	case LcCalculationMode::InductanceFromFrequencyCapacitance: {
		if (!is_positive(input.frequency_mhz))
			return invalid_result("Frequency must be greater than zero.");
		if (!is_positive(input.capacitance_pf))
			return invalid_result("Capacitance must be greater than zero.");

		const double frequency_hz = input.frequency_mhz * 1000000.0;
		const double capacitance_f = input.capacitance_pf * 0.000000000001;
		const double inductance_h = 1.0 / ((2.0 * PI * frequency_hz) * (2.0 * PI * frequency_hz) * capacitance_f);

		result.ok = true;
		result.capacitance_pf = input.capacitance_pf;
		result.inductance_uh = inductance_h * 1000000.0;
		set_frequency_fields(result, frequency_hz);
		return result;
	}
	case LcCalculationMode::CapacitanceFromFrequencyInductance: {
		if (!is_positive(input.frequency_mhz))
			return invalid_result("Frequency must be greater than zero.");
		if (!is_positive(input.inductance_uh))
			return invalid_result("Inductance must be greater than zero.");

		const double frequency_hz = input.frequency_mhz * 1000000.0;
		const double inductance_h = input.inductance_uh * 0.000001;
		const double capacitance_f = 1.0 / ((2.0 * PI * frequency_hz) * (2.0 * PI * frequency_hz) * inductance_h);

		result.ok = true;
		result.inductance_uh = input.inductance_uh;
		result.capacitance_pf = capacitance_f * 1000000000000.0;
		set_frequency_fields(result, frequency_hz);
		return result;
	}
	}

	return invalid_result("Unsupported LC calculation mode.");
}

}
