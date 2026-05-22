// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/coil_calculator.cpp

#include "coil_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

constexpr double INCHES_PER_METRE = 39.37007874015748;

CoilCalculationResult
invalid_result(const std::string &error)
{
	CoilCalculationResult result;

	result.error = error;

	return result;
}

}

CoilCalculationResult
calculate_air_core_solenoid(const CoilCalculationInput &input)
{
	if (!std::isfinite(input.turns) || input.turns <= 0.0)
		return invalid_result("Turns must be greater than zero.");
	if (!std::isfinite(input.diameter_m) || input.diameter_m <= 0.0)
		return invalid_result("Coil diameter must be greater than zero.");
	if (!std::isfinite(input.length_m) || input.length_m <= 0.0)
		return invalid_result("Coil length must be greater than zero.");

	const double diameter_in = input.diameter_m * INCHES_PER_METRE;
	const double length_in = input.length_m * INCHES_PER_METRE;

	CoilCalculationResult result;
	result.ok = true;
	result.inductance_uh = (input.turns * input.turns * diameter_in * diameter_in)
		/ ((18.0 * diameter_in) + (40.0 * length_in));
	result.note = "Wheeler single-layer air-core approximation. It does not account for wire diameter, spacing, nearby metal, ferrite cores, self-capacitance, or high-power heating.";

	return result;
}

}
