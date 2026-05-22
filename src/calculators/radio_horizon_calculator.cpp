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
horizon_km(double height_m)
{
	return std::sqrt(12.746 * height_m);
}

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
	result.tx_horizon_km = horizon_km(input.tx_height_m);
	result.rx_horizon_km = horizon_km(input.rx_height_m);
	result.combined_distance_km = result.tx_horizon_km + result.rx_horizon_km;
	result.note = "Geometric/radio-horizon estimate only. Real range depends on terrain, clutter, foliage, buildings, power, antenna gain, polarisation, receiver sensitivity, coax loss, mode, noise, and atmospheric conditions.";

	return result;
}

}
