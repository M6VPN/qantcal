// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/antenna_calculator.cpp

#include "antenna_calculator.h"

#include "rf_units.h"

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
	case AntennaType::InvertedVee:
		return 0.5;
	case AntennaType::QuarterWaveVertical:
		return 0.25;
	case AntennaType::FullWaveLoop:
		return 1.0;
	case AntennaType::RandomWire:
		return 0.0;
	}

	return 0.0;
}

bool
is_reasonable_factor(double shortening_factor)
{
	return std::isfinite(shortening_factor)
		&& shortening_factor >= MIN_WIRE_FACTOR
		&& shortening_factor <= MAX_WIRE_FACTOR;
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

	result.antenna_type = input.antenna_type;
	result.frequency_mhz = input.frequency_mhz;
	result.total_length_m = input.length_m;
	result.total_length_ft = metres_to_feet(input.length_m);
	result.shortening_factor = input.shortening_factor;
	result.error = error;

	return result;
}

std::string
counterpoise_note_for_type(AntennaType antenna_type)
{
	switch (antenna_type) {
	case AntennaType::QuarterWaveVertical:
		return "Radials or a counterpoise are required. Radial layout is not calculated in this pass.";
	case AntennaType::EndFedHalfWave:
	case AntennaType::RandomWire:
		return "A counterpoise, station ground path, or installation-specific return path may affect tuning and RF behaviour.";
	case AntennaType::HalfWaveDipole:
	case AntennaType::FullWaveLoop:
	case AntennaType::InvertedVee:
		return "No separate radial system is calculated for this antenna type.";
	}

	return "";
}

std::string
matching_note_for_type(AntennaType antenna_type)
{
	switch (antenna_type) {
	case AntennaType::HalfWaveDipole:
	case AntennaType::InvertedVee:
		return "Feed-point impedance depends on height, angle, nearby objects, and feed arrangement.";
	case AntennaType::QuarterWaveVertical:
		return "Matching depends strongly on the radial or counterpoise system and ground conditions.";
	case AntennaType::EndFedHalfWave:
		return "An EFHW normally needs a suitable matching transformer. Installation details affect tuning.";
	case AntennaType::FullWaveLoop:
		return "Loop impedance depends on shape, height, feed point, and surroundings.";
	case AntennaType::RandomWire:
		return "Random-wire operation needs tuner and counterpoise modelling before useful length guidance can be claimed.";
	}

	return "";
}

std::string
trimming_note_for_type(AntennaType antenna_type)
{
	switch (antenna_type) {
	case AntennaType::InvertedVee:
		return "Starting electrical length matches a dipole. Apex angle, height, and end effects alter real tuning.";
	case AntennaType::RandomWire:
		return "Do not treat this as a resonant cut length. Model and test the full matching and counterpoise system.";
	case AntennaType::HalfWaveDipole:
	case AntennaType::QuarterWaveVertical:
	case AntennaType::EndFedHalfWave:
	case AntennaType::FullWaveLoop:
		return "Starting dimension only. Trim and verify with an analyser, VNA, or SWR checks in the final installation.";
	}

	return "Starting dimension only. Trim and verify in the final installation.";
}

void
populate_type_lengths(AntennaCalculationResult &result, AntennaType antenna_type, double length_m)
{
	switch (antenna_type) {
	case AntennaType::HalfWaveDipole:
	case AntennaType::InvertedVee:
		result.total_length_m = length_m;
		result.total_length_ft = metres_to_feet(length_m);
		result.leg_length_m = length_m / 2.0;
		result.leg_length_ft = metres_to_feet(result.leg_length_m);
		break;
	case AntennaType::QuarterWaveVertical:
		result.total_length_m = length_m;
		result.total_length_ft = metres_to_feet(length_m);
		result.radiator_length_m = length_m;
		result.radiator_length_ft = metres_to_feet(length_m);
		break;
	case AntennaType::EndFedHalfWave:
	case AntennaType::FullWaveLoop:
		result.total_length_m = length_m;
		result.total_length_ft = metres_to_feet(length_m);
		break;
	case AntennaType::RandomWire:
		break;
	}
}

void
populate_notes(AntennaCalculationResult &result)
{
	result.counterpoise_note = counterpoise_note_for_type(result.antenna_type);
	result.matching_note = matching_note_for_type(result.antenna_type);
	result.trimming_note = trimming_note_for_type(result.antenna_type);
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
	case AntennaType::InvertedVee:
		return "Inverted Vee";
	case AntennaType::RandomWire:
		return "Random wire";
	}

	return "Unknown";
}

AntennaCalculationResult
calculate_antenna(const AntennaCalculationInput &input)
{
	const double wave_ratio = base_wave_ratio(input.antenna_type);

	if (!is_reasonable_factor(input.shortening_factor))
		return invalid_result(input, "Shortening factor must be between 0.50 and 1.00.");

	if (input.antenna_type == AntennaType::RandomWire)
		return invalid_result(input, "Random-wire resonance is not implemented. Matching and counterpoise modelling are needed first.");

	AntennaCalculationResult result;
	result.ok = true;
	result.antenna_type = input.antenna_type;
	result.shortening_factor = input.shortening_factor;
	populate_notes(result);

	if (input.design_mode == DesignMode::FrequencyToLength) {
		if (!is_reasonable_frequency(input.frequency_mhz))
			return invalid_result(input, "Frequency must be positive and within the supported scaffold range.");

		const double wavelength_m = wavelength_from_frequency_m(input.frequency_mhz);
		const double length_m = wavelength_m * wave_ratio * input.shortening_factor;

		result.frequency_mhz = input.frequency_mhz;
		result.wavelength_m = wavelength_m;
		populate_type_lengths(result, input.antenna_type, length_m);

		return result;
	}

	if (!is_reasonable_length(input.length_m))
		return invalid_result(input, "Length must be positive and within the supported scaffold range.");

	const double wavelength_m = input.length_m / (wave_ratio * input.shortening_factor);

	result.frequency_mhz = frequency_from_wavelength_mhz(wavelength_m);
	result.wavelength_m = wavelength_m;
	populate_type_lengths(result, input.antenna_type, input.length_m);

	return result;
}

double
frequency_from_wavelength_mhz(double wavelength_m)
{
	if (!std::isfinite(wavelength_m) || wavelength_m <= 0.0)
		return 0.0;

	return hz_to_mhz(SPEED_OF_LIGHT_MPS / wavelength_m);
}

double
wavelength_from_frequency_m(double frequency_mhz)
{
	if (!std::isfinite(frequency_mhz) || frequency_mhz <= 0.0)
		return 0.0;

	return SPEED_OF_LIGHT_MPS / mhz_to_hz(frequency_mhz);
}

}
