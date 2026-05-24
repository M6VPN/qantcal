// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/loading_coil_calculator.cpp

#include "loading_coil_calculator.h"

#include "antenna_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

constexpr double PI = 3.14159265358979323846;

bool
is_valid_non_negative(double value)
{
	return std::isfinite(value) && value >= 0.0;
}

}

LoadingCoilResult
calculate_loading_coil(const LoadingCoilInput &input)
{
	LoadingCoilResult result;

	if (!std::isfinite(input.frequency_mhz) || input.frequency_mhz <= 0.0) {
		result.error_message = QStringLiteral("Frequency must be positive.");
		return result;
	}
	if (!is_valid_non_negative(input.physical_vertical_height_metres) || !is_valid_non_negative(input.top_loading_length_metres)) {
		result.error_message = QStringLiteral("Physical antenna lengths cannot be negative.");
		return result;
	}
	if (input.has_estimated_capacitance && (!std::isfinite(input.estimated_capacitance_pf) || input.estimated_capacitance_pf <= 0.0)) {
		result.error_message = QStringLiteral("Estimated capacitance must be positive when supplied.");
		return result;
	}

	result.ok = true;
	result.wavelength_metres = wavelength_from_frequency_m(input.frequency_mhz);
	result.electrical_height_ratio = result.wavelength_metres > 0.0
		? input.physical_vertical_height_metres / result.wavelength_metres
		: 0.0;
	result.loading_likely_required = result.electrical_height_ratio < 0.25;
	result.warnings << QStringLiteral("This helper only resonates a supplied capacitance. It is not a full LF/MF antenna model.");
	result.warnings << QStringLiteral("Real loading coil design needs measured antenna capacitance, coil Q, current and voltage rating, losses, insulation spacing, weatherproofing, and matching-network design.");

	if (!input.has_estimated_capacitance) {
		result.warnings << QStringLiteral("No capacitance supplied, so qantcal does not estimate a loading inductance.");
		return result;
	}

	const double frequency_hz = mhz_to_hz(input.frequency_mhz);
	const double capacitance_f = input.estimated_capacitance_pf * 1.0e-12;
	const double omega = 2.0 * PI * frequency_hz;

	result.inductance_h = 1.0 / (omega * omega * capacitance_f);
	result.inductance_uh = result.inductance_h * 1.0e6;
	result.inductance_mh = result.inductance_h * 1.0e3;
	result.has_inductance = true;

	return result;
}

}
