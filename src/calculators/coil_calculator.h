// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/coil_calculator.h

#ifndef QANTCAL_CALCULATORS_COIL_CALCULATOR_H
#define QANTCAL_CALCULATORS_COIL_CALCULATOR_H

#include <string>

namespace qantcal::calculators {

struct CoilCalculationInput {
	double diameter_m = 0.0;
	double length_m = 0.0;
	double turns = 0.0;
};

struct CoilCalculationResult {
	bool ok = false;
	double inductance_uh = 0.0;
	std::string error;
	std::string note;
};

CoilCalculationResult calculate_air_core_solenoid(const CoilCalculationInput &input);

}

#endif
