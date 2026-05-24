// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/coax_loss_calculator.cpp

#include "coax_loss_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

bool
is_valid_non_negative(double value)
{
	return std::isfinite(value) && value >= 0.0;
}

}

CoaxLossResult
calculate_coax_loss(const CoaxLossInput &input)
{
	CoaxLossResult result;

	if (!std::isfinite(input.frequency_mhz) || input.frequency_mhz <= 0.0) {
		result.error_message = QStringLiteral("Frequency must be positive.");
		return result;
	}
	if (!is_valid_non_negative(input.length_metres)) {
		result.error_message = QStringLiteral("Coax length cannot be negative.");
		return result;
	}
	if (!is_valid_non_negative(input.loss_db_per_100m)) {
		result.error_message = QStringLiteral("Coax loss cannot be negative.");
		return result;
	}
	if (!is_valid_non_negative(input.input_power_watts)) {
		result.error_message = QStringLiteral("Input power cannot be negative.");
		return result;
	}
	if (!std::isfinite(input.swr) || input.swr < 1.0) {
		result.error_message = QStringLiteral("SWR must be 1.0 or greater.");
		return result;
	}

	result.ok = true;
	result.matched_loss_db = input.loss_db_per_100m * input.length_metres / 100.0;

	const double matched_power_ratio = std::pow(10.0, -result.matched_loss_db / 10.0);
	const double reflection_coefficient = (input.swr - 1.0) / (input.swr + 1.0);
	const double denominator = 1.0 - (reflection_coefficient * reflection_coefficient * matched_power_ratio * matched_power_ratio);
	double delivered_ratio = matched_power_ratio;

	if (denominator > 0.0) {
		delivered_ratio = matched_power_ratio * (1.0 - reflection_coefficient * reflection_coefficient) / denominator;
		if (delivered_ratio > 0.0)
			result.total_loss_db = -10.0 * std::log10(delivered_ratio);
	}

	result.additional_swr_loss_db = result.total_loss_db - result.matched_loss_db;
	if (result.additional_swr_loss_db < 0.0 && result.additional_swr_loss_db > -0.000001)
		result.additional_swr_loss_db = 0.0;
	result.delivered_power_watts = input.input_power_watts * delivered_ratio;
	result.loss_percent = input.input_power_watts > 0.0
		? (1.0 - delivered_ratio) * 100.0
		: 0.0;
	result.note = QStringLiteral("Uses user-supplied matched coax loss at the operating frequency. Real loss depends on cable type, frequency, connectors, installation, age, water ingress, and manufacturer data.");

	if (result.matched_loss_db >= 6.0)
		result.warnings << QStringLiteral("Matched coax loss is very high; most input power is lost before the antenna.");
	else if (result.matched_loss_db >= 3.0)
		result.warnings << QStringLiteral("Matched coax loss is significant.");
	if (input.swr >= 3.0)
		result.warnings << QStringLiteral("High SWR increases line loss and may stress transmitters or matching networks.");
	if (input.loss_db_per_100m <= 0.0 && input.length_metres > 0.0)
		result.warnings << QStringLiteral("Loss per 100 m is zero, so this is an idealised lossless-line result.");

	return result;
}

}
