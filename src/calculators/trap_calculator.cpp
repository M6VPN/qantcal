// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/trap_calculator.cpp

#include "trap_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

constexpr double PI = 3.14159265358979323846;

bool
is_positive(double value)
{
	return std::isfinite(value) && value > 0.0;
}

TrapCalculationResult
invalid_result(const QString &error_message)
{
	TrapCalculationResult result;

	result.error_message = error_message;

	return result;
}

double
capacitance_f_from_pf(double capacitance_pf)
{
	return capacitance_pf * 0.000000000001;
}

double
frequency_hz_from_mhz(double frequency_mhz)
{
	return frequency_mhz * 1000000.0;
}

double
inductance_h_from_uh(double inductance_uh)
{
	return inductance_uh * 0.000001;
}

double
capacitive_reactance_ohms(double frequency_hz, double capacitance_f)
{
	return 1.0 / (2.0 * PI * frequency_hz * capacitance_f);
}

double
inductive_reactance_ohms(double frequency_hz, double inductance_h)
{
	return 2.0 * PI * frequency_hz * inductance_h;
}

void
set_frequency_fields(TrapCalculationResult &result, double frequency_hz)
{
	result.frequency_hz = frequency_hz;
	result.frequency_khz = frequency_hz / 1000.0;
	result.frequency_mhz = frequency_hz / 1000000.0;
}

void
add_practical_warnings(TrapCalculationResult &result, const TrapCalculationInput &input)
{
	if (result.capacitance_pf < 1.0)
		result.warnings << QStringLiteral("Trap capacitance is very small; stray capacitance can dominate the design.");
	else if (result.capacitance_pf > 10000.0)
		result.warnings << QStringLiteral("Trap capacitance is very large for ordinary RF trap construction.");
	if (result.inductance_uh < 0.05)
		result.warnings << QStringLiteral("Trap inductance is very small; lead length and construction geometry can dominate the design.");
	else if (result.inductance_uh > 10000.0)
		result.warnings << QStringLiteral("Trap inductance is very large for ordinary RF trap construction.");
	if (!is_positive(input.operating_frequency_mhz))
		return;

	const double delta_ratio = std::fabs(input.operating_frequency_mhz - result.frequency_mhz) / result.frequency_mhz;
	if (delta_ratio <= 0.02)
		result.warnings << QStringLiteral("Operating frequency is extremely close to trap resonance; real trap voltage, heating, and bandwidth depend on Q and losses.");
	else if (delta_ratio <= 0.10)
		result.warnings << QStringLiteral("Operating frequency is close to trap resonance; verify trap behaviour with measured components.");
}

void
set_operating_reactance(TrapCalculationResult &result, const TrapCalculationInput &input)
{
	if (!is_positive(input.operating_frequency_mhz))
		return;

	const double operating_frequency_hz = frequency_hz_from_mhz(input.operating_frequency_mhz);
	const double inductance_h = inductance_h_from_uh(result.inductance_uh);
	const double capacitance_f = capacitance_f_from_pf(result.capacitance_pf);

	result.has_operating_reactance = true;
	result.operating_frequency_mhz = input.operating_frequency_mhz;
	result.operating_inductive_reactance_ohms = inductive_reactance_ohms(operating_frequency_hz, inductance_h);
	result.operating_capacitive_reactance_ohms = capacitive_reactance_ohms(operating_frequency_hz, capacitance_f);
}

TrapCalculationResult
finalise_result(TrapCalculationResult result, const TrapCalculationInput &input)
{
	const double frequency_hz = frequency_hz_from_mhz(result.frequency_mhz);
	const double inductance_h = inductance_h_from_uh(result.inductance_uh);

	result.ok = true;
	result.reactance_ohms = inductive_reactance_ohms(frequency_hz, inductance_h);
	result.note = QStringLiteral("First-pass parallel LC trap helper. It calculates ideal component values only and does not model Q, trap loss, voltage stress, heating, enclosure effects, or antenna element interaction.");
	set_frequency_fields(result, frequency_hz);
	set_operating_reactance(result, input);
	add_practical_warnings(result, input);

	return result;
}

}

TrapCalculationResult
calculate_trap(const TrapCalculationInput &input)
{
	TrapCalculationResult result;

	switch (input.mode) {
	case TrapCalculationMode::FrequencyFromLC: {
		if (!is_positive(input.inductance_uh))
			return invalid_result(QStringLiteral("Inductance must be greater than zero."));
		if (!is_positive(input.capacitance_pf))
			return invalid_result(QStringLiteral("Capacitance must be greater than zero."));

		const double inductance_h = inductance_h_from_uh(input.inductance_uh);
		const double capacitance_f = capacitance_f_from_pf(input.capacitance_pf);
		const double frequency_hz = 1.0 / (2.0 * PI * std::sqrt(inductance_h * capacitance_f));

		result.inductance_uh = input.inductance_uh;
		result.capacitance_pf = input.capacitance_pf;
		result.frequency_mhz = frequency_hz / 1000000.0;
		return finalise_result(result, input);
	}
	case TrapCalculationMode::InductanceFromFrequencyCapacitance: {
		if (!is_positive(input.frequency_mhz))
			return invalid_result(QStringLiteral("Frequency must be greater than zero."));
		if (!is_positive(input.capacitance_pf))
			return invalid_result(QStringLiteral("Capacitance must be greater than zero."));

		const double frequency_hz = frequency_hz_from_mhz(input.frequency_mhz);
		const double capacitance_f = capacitance_f_from_pf(input.capacitance_pf);
		const double angular_frequency = 2.0 * PI * frequency_hz;

		result.frequency_mhz = input.frequency_mhz;
		result.capacitance_pf = input.capacitance_pf;
		result.inductance_uh = 1000000.0 / (angular_frequency * angular_frequency * capacitance_f);
		return finalise_result(result, input);
	}
	case TrapCalculationMode::CapacitanceFromFrequencyInductance: {
		if (!is_positive(input.frequency_mhz))
			return invalid_result(QStringLiteral("Frequency must be greater than zero."));
		if (!is_positive(input.inductance_uh))
			return invalid_result(QStringLiteral("Inductance must be greater than zero."));

		const double frequency_hz = frequency_hz_from_mhz(input.frequency_mhz);
		const double inductance_h = inductance_h_from_uh(input.inductance_uh);
		const double angular_frequency = 2.0 * PI * frequency_hz;

		result.frequency_mhz = input.frequency_mhz;
		result.inductance_uh = input.inductance_uh;
		result.capacitance_pf = 1000000000000.0 / (angular_frequency * angular_frequency * inductance_h);
		return finalise_result(result, input);
	}
	}

	return invalid_result(QStringLiteral("Unsupported trap calculation mode."));
}

}
