// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/antenna_calculator.h

#ifndef QANTCAL_CALCULATORS_ANTENNA_CALCULATOR_H
#define QANTCAL_CALCULATORS_ANTENNA_CALCULATOR_H

#include <string>
#include <vector>

namespace qantcal::calculators {

constexpr double SPEED_OF_LIGHT_MPS = 299792458.0;
constexpr double DEFAULT_WIRE_FACTOR = 0.95;
constexpr double MIN_WIRE_FACTOR = 0.50;
constexpr double MAX_WIRE_FACTOR = 1.00;

enum class AntennaType {
	HalfWaveDipole,
	QuarterWaveVertical,
	EndFedHalfWave,
	FullWaveLoop,
	InvertedVee,
	RandomWire,
	Yagi
};

enum class DesignMode {
	FrequencyToLength,
	LengthToFrequency
};

struct AntennaCalculationInput {
	AntennaType antenna_type = AntennaType::HalfWaveDipole;
	DesignMode design_mode = DesignMode::FrequencyToLength;
	double frequency_mhz = 0.0;
	double length_m = 0.0;
	double shortening_factor = DEFAULT_WIRE_FACTOR;
};

struct AntennaCalculationResult {
	bool ok = false;
	AntennaType antenna_type = AntennaType::HalfWaveDipole;
	double frequency_mhz = 0.0;
	double wavelength_m = 0.0;
	double total_length_m = 0.0;
	double total_length_ft = 0.0;
	double leg_length_m = 0.0;
	double leg_length_ft = 0.0;
	double radiator_length_m = 0.0;
	double radiator_length_ft = 0.0;
	double shortening_factor = DEFAULT_WIRE_FACTOR;
	std::string counterpoise_note;
	std::string error;
	std::string matching_note;
	std::string trimming_note;
	std::vector<std::string> warnings;
};

AntennaCalculationResult calculate_antenna(const AntennaCalculationInput &input);
const char *antenna_type_label(AntennaType antenna_type);
double feet_to_metres(double feet);
double frequency_from_wavelength_mhz(double wavelength_m);
double hz_to_mhz(double hz);
double metres_to_feet(double metres);
double mhz_to_hz(double mhz);
double wavelength_from_frequency_m(double frequency_mhz);

}

#endif
