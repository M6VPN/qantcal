// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/radio_horizon_calculator.h

#ifndef QANTCAL_CALCULATORS_RADIO_HORIZON_CALCULATOR_H
#define QANTCAL_CALCULATORS_RADIO_HORIZON_CALCULATOR_H

#include <string>

namespace qantcal::calculators {

struct RadioHorizonInput {
	double rx_height_m = 0.0;
	double tx_height_m = 0.0;
};

struct RadioHorizonResult {
	bool ok = false;
	double combined_distance_km = 0.0;
	double rx_horizon_km = 0.0;
	double tx_horizon_km = 0.0;
	std::string error;
	std::string note;
};

RadioHorizonResult calculate_radio_horizon(const RadioHorizonInput &input);

}

#endif
