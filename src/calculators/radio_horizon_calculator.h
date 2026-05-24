// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/radio_horizon_calculator.h

#ifndef QANTCAL_CALCULATORS_RADIO_HORIZON_CALCULATOR_H
#define QANTCAL_CALCULATORS_RADIO_HORIZON_CALCULATOR_H

#include <string>

namespace qantcal::calculators {

enum class RadioHorizonModel {
	Geometric,
	EffectiveEarthFourThirds
};

struct RadioHorizonInput {
	double rx_height_m = 0.0;
	double tx_height_m = 0.0;
	RadioHorizonModel model = RadioHorizonModel::EffectiveEarthFourThirds;
};

struct RadioHorizonResult {
	bool ok = false;
	double combined_distance_km = 0.0;
	double rx_horizon_km = 0.0;
	double tx_horizon_km = 0.0;
	std::string error;
	std::string model_label;
	std::string note;
};

RadioHorizonResult calculate_radio_horizon(const RadioHorizonInput &input);
const char *radio_horizon_model_label(RadioHorizonModel model);

}

#endif
