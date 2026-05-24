// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/lf_mf_antenna_calculator.cpp

#include "lf_mf_antenna_calculator.h"

#include "antenna_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

bool
is_valid_non_negative(double value)
{
	return std::isfinite(value) && value >= 0.0;
}

QString
format_ratio(double ratio)
{
	return QStringLiteral("%1 lambda").arg(ratio, 0, 'f', 4);
}

void
append_standard_warnings(LfMfAntennaResult &result)
{
	result.warnings << QStringLiteral("LF/MF dimensions are starting guidance only, not efficiency, ERP/EIRP, field strength, bandwidth, Q, or matching predictions.");
	result.warnings << QStringLiteral("Ground/counterpoise loss, loading coil Q, nearby conductors, soil, supports, and matching networks can dominate real LF/MF performance.");
	result.warnings << QStringLiteral("High RF voltages can appear across loading coils and at antenna ends. Outdoor antennas require safe supports, weatherproofing, static/lightning precautions, and legal siting.");
}

}

LfMfAntennaResult
calculate_lf_mf_antenna(const LfMfAntennaInput &input)
{
	LfMfAntennaResult result;

	if (!std::isfinite(input.frequency_mhz) || input.frequency_mhz <= 0.0) {
		result.error_message = QStringLiteral("Frequency must be positive.");
		return result;
	}
	if (!is_valid_non_negative(input.vertical_height_metres) || !is_valid_non_negative(input.horizontal_or_top_length_metres)) {
		result.error_message = QStringLiteral("Physical antenna lengths cannot be negative.");
		return result;
	}
	if (input.has_estimated_capacitance && (!std::isfinite(input.estimated_capacitance_pf) || input.estimated_capacitance_pf <= 0.0)) {
		result.error_message = QStringLiteral("Estimated capacitance must be positive when supplied.");
		return result;
	}

	result.ok = true;
	result.frequency_mhz = input.frequency_mhz;
	result.frequency_khz = input.frequency_mhz * 1000.0;
	result.wavelength_metres = wavelength_from_frequency_m(input.frequency_mhz);
	result.quarter_wave_metres = result.wavelength_metres / 4.0;
	result.half_wave_metres = result.wavelength_metres / 2.0;
	result.full_wave_metres = result.wavelength_metres;
	result.electrical_height_ratio = result.wavelength_metres > 0.0
		? input.vertical_height_metres / result.wavelength_metres
		: 0.0;
	result.total_wire_length_metres = input.vertical_height_metres + input.horizontal_or_top_length_metres;
	result.total_wire_ratio = result.wavelength_metres > 0.0
		? result.total_wire_length_metres / result.wavelength_metres
		: 0.0;
	append_standard_warnings(result);

	result.dimensions << QStringLiteral("Wavelength: %1").arg(QString::fromStdString(format_length(result.wavelength_metres, input.preferred_length_unit)));
	result.dimensions << QStringLiteral("Quarter-wave reference: %1").arg(QString::fromStdString(format_length(result.quarter_wave_metres, input.preferred_length_unit)));
	result.dimensions << QStringLiteral("Half-wave reference: %1").arg(QString::fromStdString(format_length(result.half_wave_metres, input.preferred_length_unit)));
	result.dimensions << QStringLiteral("Full-wave reference: %1").arg(QString::fromStdString(format_length(result.full_wave_metres, input.preferred_length_unit)));

	switch (input.design_type) {
	case LfMfDesignType::FullSizeReference:
		result.notes << QStringLiteral("Full-size LF/MF antennas are usually impractical at ordinary sites. Treat these as reference dimensions.");
		break;
	case LfMfDesignType::ShortLoadedVertical:
		result.loading_likely_required = true;
		result.dimensions << QStringLiteral("Physical vertical height: %1").arg(QString::fromStdString(format_length(input.vertical_height_metres, input.preferred_length_unit)));
		result.dimensions << QStringLiteral("Electrical height: %1").arg(format_ratio(result.electrical_height_ratio));
		result.notes << QStringLiteral("A loading coil and matching or tuning network are likely required.");
		if (result.electrical_height_ratio < 0.05)
			result.warnings << QStringLiteral("Vertical height is below 0.05 wavelength, so this is an electrically short antenna.");
		break;
	case LfMfDesignType::InvertedL:
		result.loading_likely_required = true;
		result.dimensions << QStringLiteral("Vertical section: %1").arg(QString::fromStdString(format_length(input.vertical_height_metres, input.preferred_length_unit)));
		result.dimensions << QStringLiteral("Horizontal/top wire: %1").arg(QString::fromStdString(format_length(input.horizontal_or_top_length_metres, input.preferred_length_unit)));
		result.dimensions << QStringLiteral("Total wire length: %1").arg(QString::fromStdString(format_length(result.total_wire_length_metres, input.preferred_length_unit)));
		result.dimensions << QStringLiteral("Total wire ratio: %1").arg(format_ratio(result.total_wire_ratio));
		result.notes << QStringLiteral("Top loading can reduce required loading inductance, but base loading or a matching network is still likely.");
		break;
	case LfMfDesignType::TopLoadedT:
		result.loading_likely_required = true;
		result.dimensions << QStringLiteral("Vertical section: %1").arg(QString::fromStdString(format_length(input.vertical_height_metres, input.preferred_length_unit)));
		result.dimensions << QStringLiteral("Total top-hat length: %1").arg(QString::fromStdString(format_length(input.horizontal_or_top_length_metres, input.preferred_length_unit)));
		result.notes << QStringLiteral("Top loading reduces required loading inductance but does not remove the need for tuning.");
		break;
	case LfMfDesignType::ReceiveOnlyCompact:
		result.receive_only = true;
		result.notes << QStringLiteral("Receive-only compact options include active E-field probes, tuned loops, magnetic loop receive antennas, and ferrite antennas.");
		result.notes << QStringLiteral("Do not use this receive-only guidance for transmitting.");
		break;
	}

	LoadingCoilInput coil_input;
	coil_input.frequency_mhz = input.frequency_mhz;
	coil_input.physical_vertical_height_metres = input.vertical_height_metres;
	coil_input.top_loading_length_metres = input.horizontal_or_top_length_metres;
	coil_input.estimated_capacitance_pf = input.estimated_capacitance_pf;
	coil_input.has_estimated_capacitance = input.has_estimated_capacitance;
	result.loading_coil = calculate_loading_coil(coil_input);
	if (result.loading_coil.ok && result.loading_coil.loading_likely_required)
		result.loading_likely_required = true;

	return result;
}

QString
lf_mf_design_type_key(LfMfDesignType design_type)
{
	switch (design_type) {
	case LfMfDesignType::FullSizeReference:
		return QStringLiteral("full_size_reference");
	case LfMfDesignType::ShortLoadedVertical:
		return QStringLiteral("short_loaded_vertical");
	case LfMfDesignType::InvertedL:
		return QStringLiteral("inverted_l");
	case LfMfDesignType::TopLoadedT:
		return QStringLiteral("top_loaded_t");
	case LfMfDesignType::ReceiveOnlyCompact:
		return QStringLiteral("receive_only_compact");
	}

	return QStringLiteral("full_size_reference");
}

QString
lf_mf_design_type_label(LfMfDesignType design_type)
{
	switch (design_type) {
	case LfMfDesignType::FullSizeReference:
		return QStringLiteral("Full-size reference");
	case LfMfDesignType::ShortLoadedVertical:
		return QStringLiteral("Short loaded vertical");
	case LfMfDesignType::InvertedL:
		return QStringLiteral("Inverted-L");
	case LfMfDesignType::TopLoadedT:
		return QStringLiteral("T / top-loaded vertical");
	case LfMfDesignType::ReceiveOnlyCompact:
		return QStringLiteral("Receive-only compact antenna");
	}

	return QStringLiteral("Full-size reference");
}

LfMfDesignType
lf_mf_design_type_from_key(const QString &key)
{
	if (key == QStringLiteral("short_loaded_vertical"))
		return LfMfDesignType::ShortLoadedVertical;
	if (key == QStringLiteral("inverted_l"))
		return LfMfDesignType::InvertedL;
	if (key == QStringLiteral("top_loaded_t"))
		return LfMfDesignType::TopLoadedT;
	if (key == QStringLiteral("receive_only_compact"))
		return LfMfDesignType::ReceiveOnlyCompact;

	return LfMfDesignType::FullSizeReference;
}

}
