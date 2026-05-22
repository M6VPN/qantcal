// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/antenna_calculator.cpp

#include "antenna_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

constexpr double MIN_FREQUENCY_MHZ = 0.001;
constexpr double MAX_FREQUENCY_MHZ = 300000.0;
constexpr double MIN_LENGTH_M = 0.001;
constexpr double MAX_LENGTH_M = 10000.0;

double
base_wave_ratio(AntennaType antenna_type)
{
	switch (antenna_type) {
	case AntennaType::HalfWaveDipole:
	case AntennaType::EndFedHalfWave:
		return 0.5;
	case AntennaType::QuarterWaveVertical:
		return 0.25;
	case AntennaType::FullWaveLoop:
		return 1.0;
	case AntennaType::LongWirePlaceholder:
		return 0.0;
	}

	return 0.0;
}

bool
is_reasonable_factor(double velocity_factor)
{
	return std::isfinite(velocity_factor)
		&& velocity_factor > 0.0
		&& velocity_factor <= 1.0;
}

bool
is_reasonable_frequency(double frequency_mhz)
{
	return std::isfinite(frequency_mhz)
		&& frequency_mhz >= MIN_FREQUENCY_MHZ
		&& frequency_mhz <= MAX_FREQUENCY_MHZ;
}

bool
is_reasonable_length(double length_m)
{
	return std::isfinite(length_m)
		&& length_m >= MIN_LENGTH_M
		&& length_m <= MAX_LENGTH_M;
}

AntennaCalculationResult
invalid_result(const AntennaCalculationInput &input, const std::string &error)
{
	AntennaCalculationResult result;

	result.frequency_mhz = input.frequency_mhz;
	result.total_length_m = input.length_m;
	result.velocity_factor = input.velocity_factor;
	result.error = error;

	return result;
}

std::string
note_for_type(AntennaType antenna_type)
{
	switch (antenna_type) {
	case AntennaType::HalfWaveDipole:
		return "Approximate total wire length. Each leg is half the total. Trim and measure in the final installation.";
	case AntennaType::QuarterWaveVertical:
		return "Approximate radiator length. Radials or a counterpoise are required but not calculated in detail yet.";
	case AntennaType::EndFedHalfWave:
		return "Approximate half-wave wire length. Matching transformer, counterpoise, and installation details affect tuning.";
	case AntennaType::FullWaveLoop:
		return "Approximate loop circumference. Shape, height, feed point, and surroundings affect impedance and resonance.";
	case AntennaType::LongWirePlaceholder:
		return "Long wire / random wire design is a placeholder. A tuner, counterpoise, and installation-specific checks are required.";
	}

	return "Approximate starting dimension only.";
}

void
populate_type_lengths(AntennaCalculationResult &result, AntennaType antenna_type, double length_m)
{
	switch (antenna_type) {
	case AntennaType::HalfWaveDipole:
		result.total_length_m = length_m;
		result.leg_length_m = length_m / 2.0;
		break;
	case AntennaType::QuarterWaveVertical:
		result.total_length_m = length_m;
		result.radiator_length_m = length_m;
		break;
	case AntennaType::EndFedHalfWave:
	case AntennaType::FullWaveLoop:
		result.total_length_m = length_m;
		break;
	case AntennaType::LongWirePlaceholder:
		break;
	}
}

}

const char *
antenna_type_label(AntennaType antenna_type)
{
	switch (antenna_type) {
	case AntennaType::HalfWaveDipole:
		return "Half-wave dipole";
	case AntennaType::QuarterWaveVertical:
		return "Quarter-wave vertical";
	case AntennaType::EndFedHalfWave:
		return "End-fed half-wave";
	case AntennaType::FullWaveLoop:
		return "Full-wave loop";
	case AntennaType::LongWirePlaceholder:
		return "Long wire / random wire placeholder";
	}

	return "Unknown";
}

AntennaCalculationResult
calculate_antenna(const AntennaCalculationInput &input)
{
	const double wave_ratio = base_wave_ratio(input.antenna_type);

	if (!is_reasonable_factor(input.velocity_factor))
		return invalid_result(input, "Velocity factor must be greater than 0 and no more than 1.");

	if (wave_ratio <= 0.0)
		return invalid_result(input, "Long wire / random wire calculation is not implemented in this scaffold.");

	AntennaCalculationResult result;
	result.ok = true;
	result.velocity_factor = input.velocity_factor;
	result.note = note_for_type(input.antenna_type);

	if (input.design_mode == DesignMode::FrequencyToLength) {
		if (!is_reasonable_frequency(input.frequency_mhz))
			return invalid_result(input, "Frequency must be positive and within the supported scaffold range.");

		const double frequency_hz = input.frequency_mhz * 1000000.0;
		const double wavelength_m = SPEED_OF_LIGHT_MPS / frequency_hz;
		const double length_m = wavelength_m * wave_ratio * input.velocity_factor;

		result.frequency_mhz = input.frequency_mhz;
		result.wavelength_m = wavelength_m;
		populate_type_lengths(result, input.antenna_type, length_m);

		return result;
	}

	if (!is_reasonable_length(input.length_m))
		return invalid_result(input, "Length must be positive and within the supported scaffold range.");

	const double wavelength_m = input.length_m / (wave_ratio * input.velocity_factor);
	const double frequency_hz = SPEED_OF_LIGHT_MPS / wavelength_m;

	result.frequency_mhz = frequency_hz / 1000000.0;
	result.wavelength_m = wavelength_m;
	populate_type_lengths(result, input.antenna_type, input.length_m);

	return result;
}

}
