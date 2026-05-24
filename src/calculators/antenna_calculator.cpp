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
constexpr double PI = 3.14159265358979323846;
constexpr double LARGE_INSTALLATION_LENGTH_M = 80.0;
constexpr double IMPRACTICAL_LENGTH_M = 250.0;
constexpr double EFFECTIVELY_IMPOSSIBLE_LENGTH_M = 1000.0;
constexpr double TOLERANCE_DOMINATED_LENGTH_M = 0.05;
constexpr double LF_VLF_FREQUENCY_MHZ = 0.1;
constexpr double MICROWAVE_WARNING_FREQUENCY_MHZ = 1300.0;
constexpr double RANDOM_WIRE_MULTIPLE_TOLERANCE = 0.08;
constexpr double HALO_GAP_WAVELENGTH_RATIO = 0.015;

double
base_wave_ratio(AntennaType antenna_type)
{
	switch (antenna_type) {
	case AntennaType::HalfWaveDipole:
	case AntennaType::FoldedDipole:
	case AntennaType::Halo:
	case AntennaType::EndFedHalfWave:
	case AntennaType::InvertedVee:
		return 0.5;
	case AntennaType::QuarterWaveVertical:
		return 0.25;
	case AntennaType::FullWaveLoop:
		return 1.0;
	case AntennaType::RandomWire:
	case AntennaType::Yagi:
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
	result.conductor_length_m = input.length_m;
	result.conductor_length_ft = metres_to_feet(input.length_m);
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
	case AntennaType::FoldedDipole:
	case AntennaType::FullWaveLoop:
	case AntennaType::Halo:
	case AntennaType::InvertedVee:
	case AntennaType::Yagi:
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
	case AntennaType::FoldedDipole:
		return "A two-wire folded dipole is often near 4 times the impedance of a simple dipole and normally needs balanced feed or a suitable transformer.";
	case AntennaType::Halo:
		return "A halo is a half-wave dipole bent into a near-circle. Feed and matching details, end-gap capacitance, mounting, and nearby objects affect tuning.";
	case AntennaType::QuarterWaveVertical:
		return "Matching depends strongly on the radial or counterpoise system and ground conditions.";
	case AntennaType::EndFedHalfWave:
		return "An EFHW normally needs a suitable matching transformer. Installation details affect tuning.";
	case AntennaType::FullWaveLoop:
		return "Loop impedance depends on shape, height, feed point, and surroundings.";
	case AntennaType::RandomWire:
		return "Random-wire operation normally needs an ATU, tuner, or matching network. Feed impedance depends on wire length, frequency, counterpoise, feedline, height, and surroundings.";
	case AntennaType::Yagi:
		return "Yagi driven element matching is not designed by the simple wire calculator.";
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
	case AntennaType::FoldedDipole:
	case AntennaType::Halo:
	case AntennaType::QuarterWaveVertical:
	case AntennaType::EndFedHalfWave:
	case AntennaType::FullWaveLoop:
		return "Starting dimension only. Trim and verify with an analyser, VNA, or SWR checks in the final installation.";
	case AntennaType::Yagi:
		return "Use the Yagi designer for empirical starting dimensions. Final dimensions require modelling, measuring, and trimming.";
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
		result.conductor_length_m = length_m;
		result.conductor_length_ft = metres_to_feet(length_m);
		result.leg_length_m = length_m / 2.0;
		result.leg_length_ft = metres_to_feet(result.leg_length_m);
		break;
	case AntennaType::FoldedDipole:
		result.total_length_m = length_m;
		result.total_length_ft = metres_to_feet(length_m);
		result.conductor_length_m = length_m * 2.0;
		result.conductor_length_ft = metres_to_feet(result.conductor_length_m);
		result.leg_length_m = length_m / 2.0;
		result.leg_length_ft = metres_to_feet(result.leg_length_m);
		break;
	case AntennaType::Halo:
		result.total_length_m = length_m;
		result.total_length_ft = metres_to_feet(length_m);
		result.conductor_length_m = length_m;
		result.conductor_length_ft = metres_to_feet(length_m);
		result.halo_diameter_m = length_m / PI;
		result.halo_diameter_ft = metres_to_feet(result.halo_diameter_m);
		result.halo_gap_m = result.wavelength_m * HALO_GAP_WAVELENGTH_RATIO;
		result.halo_gap_ft = metres_to_feet(result.halo_gap_m);
		break;
	case AntennaType::QuarterWaveVertical:
		result.total_length_m = length_m;
		result.total_length_ft = metres_to_feet(length_m);
		result.conductor_length_m = length_m;
		result.conductor_length_ft = metres_to_feet(length_m);
		result.radiator_length_m = length_m;
		result.radiator_length_ft = metres_to_feet(length_m);
		break;
	case AntennaType::EndFedHalfWave:
		result.total_length_m = length_m;
		result.total_length_ft = metres_to_feet(length_m);
		result.conductor_length_m = length_m;
		result.conductor_length_ft = metres_to_feet(length_m);
		break;
	case AntennaType::FullWaveLoop:
		result.total_length_m = length_m;
		result.total_length_ft = metres_to_feet(length_m);
		result.conductor_length_m = length_m;
		result.conductor_length_ft = metres_to_feet(length_m);
		result.loop_side_length_m = length_m / 4.0;
		result.loop_side_length_ft = metres_to_feet(result.loop_side_length_m);
		break;
	case AntennaType::RandomWire:
	case AntennaType::Yagi:
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

void
append_practical_warnings(AntennaCalculationResult &result)
{
	if (result.total_length_m >= EFFECTIVELY_IMPOSSIBLE_LENGTH_M) {
		result.warnings.push_back("Calculated physical length is effectively impossible for typical amateur construction.");
	} else if (result.total_length_m >= IMPRACTICAL_LENGTH_M) {
		result.warnings.push_back("Calculated physical length is impractical for ordinary sites.");
	} else if (result.total_length_m >= LARGE_INSTALLATION_LENGTH_M) {
		result.warnings.push_back("Calculated physical length needs a large installation area and substantial supports.");
	}
	if (result.total_length_m > 0.0 && result.total_length_m <= TOLERANCE_DOMINATED_LENGTH_M)
		result.warnings.push_back("Calculated physical length is very small, so construction tolerance and connector geometry can dominate.");
	if (result.frequency_mhz > 0.0 && result.frequency_mhz < LF_VLF_FREQUENCY_MHZ)
		result.warnings.push_back("LF/VLF wire dimensions are extreme; use the LF/MF guidance tool for loaded or receive-only concepts.");
	if (result.frequency_mhz >= MICROWAVE_WARNING_FREQUENCY_MHZ)
		result.warnings.push_back("Simple wire formulas are poor guidance for microwave-style construction and feed geometry.");
}

void
append_random_wire_frequency_guidance(AntennaCalculationResult &result)
{
	result.warnings.push_back("Random-wire guidance does not calculate a resonant cut length.");
	result.warnings.push_back("Choose a physical length based on the available site, tuner range, counterpoise, supports, and measured behaviour.");
}

void
append_random_wire_length_guidance(AntennaCalculationResult &result, bool check_half_wave_multiple)
{
	if (result.wavelength_m <= 0.0 || result.total_length_m <= 0.0)
		return;

	const double half_wave_m = result.wavelength_m / 2.0;
	const double half_wave_multiple = result.total_length_m / half_wave_m;
	const double nearest_multiple = std::round(half_wave_multiple);

	result.warnings.push_back("Random-wire guidance treats the supplied length as a physical wire length, not a resonant design.");
	if (check_half_wave_multiple && nearest_multiple >= 1.0 && std::fabs(half_wave_multiple - nearest_multiple) <= RANDOM_WIRE_MULTIPLE_TOLERANCE)
		result.warnings.push_back("Supplied length is close to a half-wave multiple at the reference frequency; tuner matching can become difficult.");
}

AntennaCalculationResult
calculate_random_wire(const AntennaCalculationInput &input)
{
	AntennaCalculationResult result;

	result.ok = true;
	result.antenna_type = input.antenna_type;
	result.shortening_factor = input.shortening_factor;
	populate_notes(result);

	if (input.design_mode == DesignMode::FrequencyToLength) {
		if (!is_reasonable_frequency(input.frequency_mhz))
			return invalid_result(input, "Frequency must be positive and within the supported scaffold range.");

		result.frequency_mhz = input.frequency_mhz;
		result.wavelength_m = wavelength_from_frequency_m(input.frequency_mhz);
		append_random_wire_frequency_guidance(result);
		append_practical_warnings(result);
		return result;
	}

	if (!is_reasonable_length(input.length_m))
		return invalid_result(input, "Length must be positive and within the supported scaffold range.");

	const bool has_reference_frequency = is_reasonable_frequency(input.frequency_mhz);
	const double reference_wavelength_m = has_reference_frequency
		? wavelength_from_frequency_m(input.frequency_mhz)
		: input.length_m * 2.0;

	result.total_length_m = input.length_m;
	result.total_length_ft = metres_to_feet(input.length_m);
	result.conductor_length_m = input.length_m;
	result.conductor_length_ft = metres_to_feet(input.length_m);
	result.frequency_mhz = has_reference_frequency
		? input.frequency_mhz
		: frequency_from_wavelength_mhz(reference_wavelength_m);
	result.wavelength_m = reference_wavelength_m;
	append_random_wire_length_guidance(result, has_reference_frequency);
	append_practical_warnings(result);

	return result;
}

}

const char *
antenna_type_label(AntennaType antenna_type)
{
	switch (antenna_type) {
	case AntennaType::HalfWaveDipole:
		return "Half-wave dipole";
	case AntennaType::FoldedDipole:
		return "Folded dipole";
	case AntennaType::Halo:
		return "Halo";
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
	case AntennaType::Yagi:
		return "Yagi";
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
		return calculate_random_wire(input);
	if (input.antenna_type == AntennaType::Yagi)
		return invalid_result(input, "Use the Yagi designer for Yagi starting dimensions.");

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
		append_practical_warnings(result);

		return result;
	}

	if (!is_reasonable_length(input.length_m))
		return invalid_result(input, "Length must be positive and within the supported scaffold range.");

	const double wavelength_m = input.length_m / (wave_ratio * input.shortening_factor);

	result.frequency_mhz = frequency_from_wavelength_mhz(wavelength_m);
	result.wavelength_m = wavelength_m;
	populate_type_lengths(result, input.antenna_type, input.length_m);
	append_practical_warnings(result);

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
