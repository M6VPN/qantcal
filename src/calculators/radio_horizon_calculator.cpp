// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/radio_horizon_calculator.cpp

#include "radio_horizon_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

RadioHorizonResult
invalid_result(const std::string &error)
{
	RadioHorizonResult result;

	result.error = error;

	return result;
}

double
horizon_factor(RadioHorizonModel model)
{
	switch (model) {
	case RadioHorizonModel::Geometric:
		return 12.746;
	case RadioHorizonModel::EffectiveEarthFourThirds:
		return 17.0;
	}

	return 17.0;
}

double
horizon_km(double height_m, RadioHorizonModel model)
{
	return std::sqrt(horizon_factor(model) * height_m);
}

}

const char *
radio_horizon_model_label(RadioHorizonModel model)
{
	switch (model) {
	case RadioHorizonModel::Geometric:
		return "Geometric horizon";
	case RadioHorizonModel::EffectiveEarthFourThirds:
		return "Radio horizon (4/3 effective Earth)";
	}

	return "Radio horizon (4/3 effective Earth)";
}

RadioHorizonResult
calculate_radio_horizon(const RadioHorizonInput &input)
{
	if (!std::isfinite(input.tx_height_m) || input.tx_height_m < 0.0)
		return invalid_result("Transmitting antenna height must be zero or greater.");
	if (!std::isfinite(input.rx_height_m) || input.rx_height_m < 0.0)
		return invalid_result("Receiving antenna height must be zero or greater.");

	RadioHorizonResult result;
	result.ok = true;
	result.tx_horizon_km = horizon_km(input.tx_height_m, input.model);
	result.rx_horizon_km = horizon_km(input.rx_height_m, input.model);
	result.combined_distance_km = result.tx_horizon_km + result.rx_horizon_km;
	result.model_label = radio_horizon_model_label(input.model);
	if (input.model == RadioHorizonModel::Geometric) {
		result.note = "Geometric optical-horizon estimate only. Real RF range depends on terrain, clutter, foliage, buildings, power, antenna gain, polarisation, receiver sensitivity, coax loss, mode, noise, and atmospheric conditions.";
	} else {
		result.note = "Radio-horizon estimate using a 4/3 effective Earth-radius model. Real range depends on terrain, clutter, foliage, buildings, power, antenna gain, polarisation, receiver sensitivity, coax loss, mode, noise, and atmospheric conditions.";
	}

	return result;
}

}
