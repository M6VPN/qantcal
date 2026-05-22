// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/swr_calculator.h

#ifndef QANTCAL_CALCULATORS_SWR_CALCULATOR_H
#define QANTCAL_CALCULATORS_SWR_CALCULATOR_H

#include <string>

namespace qantcal::calculators {

struct SwrCalculationInput {
	double forward_power_w = 0.0;
	double swr = 1.0;
};

struct SwrCalculationResult {
	bool ok = false;
	double delivered_power_w = 0.0;
	double reflected_percent = 0.0;
	double reflected_power_w = 0.0;
	double reflection_coefficient = 0.0;
	std::string error;
	std::string note;
};

SwrCalculationResult calculate_swr(const SwrCalculationInput &input);

}

#endif
