// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/swr_calculator.cpp

#include "swr_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

SwrCalculationResult
invalid_result(const std::string &error)
{
	SwrCalculationResult result;

	result.error = error;

	return result;
}

}

SwrCalculationResult
calculate_swr(const SwrCalculationInput &input)
{
	if (!std::isfinite(input.swr) || input.swr < 1.0)
		return invalid_result("SWR must be 1.0 or greater.");
	if (!std::isfinite(input.forward_power_w) || input.forward_power_w < 0.0)
		return invalid_result("Forward power must be zero or greater.");

	SwrCalculationResult result;
	result.ok = true;
	result.reflection_coefficient = (input.swr - 1.0) / (input.swr + 1.0);
	result.reflected_power_w = input.forward_power_w * result.reflection_coefficient * result.reflection_coefficient;
	result.reflected_percent = result.reflection_coefficient * result.reflection_coefficient * 100.0;
	result.delivered_power_w = input.forward_power_w - result.reflected_power_w;
	result.note = "Assumes a lossless line. Tuner, coax, connector, and matching losses are not modelled.";

	return result;
}

}
