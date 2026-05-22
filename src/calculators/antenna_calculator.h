// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/antenna_calculator.h

#ifndef QANTCAL_CALCULATORS_ANTENNA_CALCULATOR_H
#define QANTCAL_CALCULATORS_ANTENNA_CALCULATOR_H

#include <string>

namespace qantcal::calculators {

constexpr double SPEED_OF_LIGHT_MPS = 299792458.0;
constexpr double DEFAULT_WIRE_FACTOR = 0.95;

enum class AntennaType {
	HalfWaveDipole,
	QuarterWaveVertical,
	EndFedHalfWave,
	FullWaveLoop,
	LongWirePlaceholder
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
	double velocity_factor = DEFAULT_WIRE_FACTOR;
};

struct AntennaCalculationResult {
	bool ok = false;
	double frequency_mhz = 0.0;
	double wavelength_m = 0.0;
	double total_length_m = 0.0;
	double leg_length_m = 0.0;
	double radiator_length_m = 0.0;
	double velocity_factor = DEFAULT_WIRE_FACTOR;
	std::string error;
	std::string note;
};

AntennaCalculationResult calculate_antenna(const AntennaCalculationInput &input);
const char *antenna_type_label(AntennaType antenna_type);

}

#endif
