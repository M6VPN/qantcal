// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/lc_resonance_calculator.h

#ifndef QANTCAL_CALCULATORS_LC_RESONANCE_CALCULATOR_H
#define QANTCAL_CALCULATORS_LC_RESONANCE_CALCULATOR_H

#include <string>

namespace qantcal::calculators {

enum class LcCalculationMode {
	FrequencyFromLC,
	InductanceFromFrequencyCapacitance,
	CapacitanceFromFrequencyInductance
};

struct LcResonanceInput {
	LcCalculationMode mode = LcCalculationMode::FrequencyFromLC;
	double capacitance_pf = 0.0;
	double frequency_mhz = 0.0;
	double inductance_uh = 0.0;
};

struct LcResonanceResult {
	bool ok = false;
	double capacitance_pf = 0.0;
	double frequency_hz = 0.0;
	double frequency_khz = 0.0;
	double frequency_mhz = 0.0;
	double inductance_uh = 0.0;
	std::string error;
};

LcResonanceResult calculate_lc_resonance(const LcResonanceInput &input);

}

#endif
