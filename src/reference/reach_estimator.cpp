// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/reach_estimator.cpp

#include "reach_estimator.h"

#include "band_reference.h"
#include "calculators/radio_horizon_calculator.h"

#include <cmath>

namespace qantcal::reference {

namespace {

ReachEstimateResult
invalid_result(const QString &message)
{
	ReachEstimateResult result;

	result.error_message = message;

	return result;
}

bool
is_vhf_or_above(double frequency_mhz)
{
	return frequency_mhz >= 50.0;
}

bool
is_hf(double frequency_mhz)
{
	return frequency_mhz >= 1.8 && frequency_mhz < 50.0;
}

QString
environment_note(EnvironmentProfile environment)
{
	switch (environment) {
	case EnvironmentProfile::Indoor:
		return QStringLiteral("Indoor operation often has high building loss and local noise.");
	case EnvironmentProfile::Urban:
		return QStringLiteral("Urban operation may be limited by buildings, clutter, and man-made noise.");
	case EnvironmentProfile::Suburban:
		return QStringLiteral("Suburban operation is mixed and depends strongly on local clutter and noise.");
	case EnvironmentProfile::Rural:
		return QStringLiteral("Rural operation may have lower noise and fewer obstructions.");
	case EnvironmentProfile::HilltopOpen:
		return QStringLiteral("Hilltop or open locations often improve line-of-sight paths.");
	}

	return QStringLiteral("Environment strongly affects practical results.");
}

}

ReachEstimateResult
estimate_reach(const ReachEstimateInput &input)
{
	if (!std::isfinite(input.frequency_mhz) || input.frequency_mhz <= 0.0)
		return invalid_result(QStringLiteral("Frequency must be positive."));
	if (!std::isfinite(input.tx_height_metres) || input.tx_height_metres < 0.0)
		return invalid_result(QStringLiteral("TX antenna height cannot be negative."));
	if (!std::isfinite(input.rx_height_metres) || input.rx_height_metres < 0.0)
		return invalid_result(QStringLiteral("RX antenna height cannot be negative."));
	if (input.has_power_watts && (!std::isfinite(input.power_watts) || input.power_watts < 0.0))
		return invalid_result(QStringLiteral("Power cannot be negative."));

	ReachEstimateResult result;
	BandReference band;
	const bool has_band = band_reference_by_frequency(input.frequency_mhz, band);
	const QString band_name = input.band_name.isEmpty() && has_band ? band.name : input.band_name;

	result.ok = true;
	result.warnings << QStringLiteral("This is rough reach guidance, not a propagation prediction.");
	result.warnings << QStringLiteral("Actual range depends on power, mode, antennas, terrain, clutter, noise, receiver performance, and conditions.");
	result.warnings << environment_note(input.environment);

	if (is_vhf_or_above(input.frequency_mhz)) {
		calculators::RadioHorizonInput horizon_input;
		horizon_input.rx_height_m = input.rx_height_metres;
		horizon_input.tx_height_m = input.tx_height_metres;
		const calculators::RadioHorizonResult horizon = calculators::calculate_radio_horizon(horizon_input);

		if (!horizon.ok)
			return invalid_result(QString::fromStdString(horizon.error));

		result.includes_radio_horizon = true;
		result.tx_horizon_km = horizon.tx_horizon_km;
		result.rx_horizon_km = horizon.rx_horizon_km;
		result.combined_horizon_km = horizon.combined_distance_km;
		result.categories << QStringLiteral("very local") << QStringLiteral("local") << QStringLiteral("line-of-sight") << QStringLiteral("highly condition-dependent");
		result.summary = QStringLiteral("%1 VHF/UHF guidance: 4/3 effective Earth radio-horizon estimate is %2 km combined. Longer paths may occur through tropospheric, sporadic-E, satellite, or other condition-dependent modes where relevant.")
			.arg(band_name.isEmpty() ? QStringLiteral("Selected") : band_name)
			.arg(result.combined_horizon_km, 0, 'f', 1);
		return result;
	}

	if (is_hf(input.frequency_mhz)) {
		result.categories << QStringLiteral("local") << QStringLiteral("regional") << QStringLiteral("long-distance/DX possible") << QStringLiteral("highly condition-dependent");
		result.summary = QStringLiteral("%1 HF guidance: use band notes for NVIS, regional skywave, and DX tendencies. qantcal does not calculate fixed kilometre predictions for HF in this pass.")
			.arg(band_name.isEmpty() ? QStringLiteral("Selected") : band_name);
		return result;
	}

	result.categories << QStringLiteral("highly condition-dependent");
	result.summary = QStringLiteral("No detailed reach guidance is available for this frequency. Check current band, mode, antenna, and propagation references.");

	return result;
}

QString
environment_profile_key(EnvironmentProfile environment)
{
	switch (environment) {
	case EnvironmentProfile::Indoor:
		return QStringLiteral("indoor");
	case EnvironmentProfile::Urban:
		return QStringLiteral("urban");
	case EnvironmentProfile::Suburban:
		return QStringLiteral("suburban");
	case EnvironmentProfile::Rural:
		return QStringLiteral("rural");
	case EnvironmentProfile::HilltopOpen:
		return QStringLiteral("hilltop_open");
	}

	return QStringLiteral("suburban");
}

QString
environment_profile_label(EnvironmentProfile environment)
{
	switch (environment) {
	case EnvironmentProfile::Indoor:
		return QStringLiteral("Indoor");
	case EnvironmentProfile::Urban:
		return QStringLiteral("Urban");
	case EnvironmentProfile::Suburban:
		return QStringLiteral("Suburban");
	case EnvironmentProfile::Rural:
		return QStringLiteral("Rural");
	case EnvironmentProfile::HilltopOpen:
		return QStringLiteral("Hilltop/open");
	}

	return QStringLiteral("Suburban");
}

EnvironmentProfile
environment_profile_from_key(const QString &key)
{
	if (key == QStringLiteral("indoor"))
		return EnvironmentProfile::Indoor;
	if (key == QStringLiteral("urban"))
		return EnvironmentProfile::Urban;
	if (key == QStringLiteral("rural"))
		return EnvironmentProfile::Rural;
	if (key == QStringLiteral("hilltop_open"))
		return EnvironmentProfile::HilltopOpen;

	return EnvironmentProfile::Suburban;
}

}
