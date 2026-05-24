// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/yagi_calculator.cpp

#include "yagi_calculator.h"

#include "antenna_calculator.h"

#include <cmath>

namespace qantcal::calculators {

namespace {

constexpr double MIN_YAGI_FACTOR = 0.85;
constexpr double MAX_YAGI_FACTOR = 1.00;
constexpr double MIN_FREQUENCY_MHZ = 0.001;
constexpr double MAX_FREQUENCY_MHZ = 300000.0;
constexpr double LARGE_YAGI_BOOM_M = 6.0;
constexpr double IMPRACTICAL_YAGI_BOOM_M = 15.0;

bool
is_valid_frequency(double frequency_mhz)
{
	return std::isfinite(frequency_mhz)
		&& frequency_mhz >= MIN_FREQUENCY_MHZ
		&& frequency_mhz <= MAX_FREQUENCY_MHZ;
}

bool
is_valid_non_negative(double value)
{
	return std::isfinite(value) && value >= 0.0;
}

bool
is_valid_yagi_factor(double factor)
{
	return std::isfinite(factor)
		&& factor >= MIN_YAGI_FACTOR
		&& factor <= MAX_YAGI_FACTOR;
}

YagiDesignResult
invalid_result(const QString &message)
{
	YagiDesignResult result;

	result.error_message = message;

	return result;
}

double
corrected_length(double length_metres, double boom_correction_metres)
{
	if (boom_correction_metres <= 0.0)
		return length_metres;
	if (length_metres - boom_correction_metres <= 0.0)
		return 0.0;

	return length_metres - boom_correction_metres;
}

double
director_floor_ratio(YagiPreset preset)
{
	if (preset == YagiPreset::LongBoom)
		return 0.88;

	return 0.90;
}

double
first_director_ratio(YagiPreset preset)
{
	if (preset == YagiPreset::LongBoom)
		return 0.94;

	return 0.95;
}

double
spacing_ratio(YagiPreset preset, int element_index)
{
	if (preset == YagiPreset::Compact) {
		if (element_index == 1)
			return 0.15;
		if (element_index == 2)
			return 0.12;
		return 0.15;
	}

	if (preset == YagiPreset::LongBoom) {
		if (element_index == 1)
			return 0.20;
		if (element_index == 2)
			return 0.20;
		return 0.25;
	}

	if (element_index == 1)
		return 0.20;
	if (element_index == 2)
		return 0.15;

	return 0.20;
}

QString
directivity_note(int element_count)
{
	if (element_count == 2)
		return QStringLiteral("Two elements give modest directivity. Gain and front-to-back ratio are not calculated.");
	if (element_count == 3)
		return QStringLiteral("Three elements are a typical small Yagi starting point. Gain and front-to-back ratio are not calculated.");

	return QStringLiteral("More elements can increase directivity, narrow bandwidth, and increase tuning sensitivity. Gain and front-to-back ratio are not calculated.");
}

}

YagiDesignResult
calculate_yagi(const YagiDesignInput &input)
{
	if (!is_valid_frequency(input.frequency_mhz))
		return invalid_result(QStringLiteral("Frequency must be positive and within the supported scaffold range."));
	if (input.element_count < 2 || input.element_count > 10)
		return invalid_result(QStringLiteral("Yagi element count must be between 2 and 10."));
	if (!is_valid_yagi_factor(input.element_shortening_factor))
		return invalid_result(QStringLiteral("Yagi element shortening factor must be between 0.85 and 1.00."));
	if (!is_valid_non_negative(input.element_diameter_metres))
		return invalid_result(QStringLiteral("Element diameter cannot be negative."));
	if (!is_valid_non_negative(input.boom_correction_metres))
		return invalid_result(QStringLiteral("Boom correction cannot be negative."));

	YagiDesignResult result;
	const double wavelength_metres = wavelength_from_frequency_m(input.frequency_mhz);
	const double base_half_wave_metres = wavelength_metres / 2.0 * input.element_shortening_factor;
	const double floor_length = base_half_wave_metres * director_floor_ratio(input.preset);
	double previous_director_length = 0.0;
	double position = 0.0;

	result.ok = true;
	result.frequency_mhz = input.frequency_mhz;
	result.wavelength_metres = wavelength_metres;

	for (int i = 0; i < input.element_count; ++i) {
		YagiElement element;
		double raw_length = base_half_wave_metres;

		if (i == 0) {
			element.label = QStringLiteral("Reflector");
			element.role = YagiElementRole::Reflector;
			raw_length = base_half_wave_metres * 1.05;
			element.notes = directivity_note(input.element_count);
		} else if (i == 1) {
			element.label = QStringLiteral("Driven");
			element.role = YagiElementRole::Driven;
			element.notes = QStringLiteral("Driven element feed and matching method are not designed in this pass.");
		} else {
			element.label = QStringLiteral("Director %1").arg(i - 1);
			element.role = YagiElementRole::Director;
			raw_length = i == 2
				? base_half_wave_metres * first_director_ratio(input.preset)
				: std::max(previous_director_length * 0.995, floor_length);
			previous_director_length = raw_length;
			element.notes = QStringLiteral("Parasitic director starting length. Modelling and trimming are required.");
		}

		if (i > 0) {
			element.spacing_from_previous_metres = wavelength_metres * spacing_ratio(input.preset, i);
			position += element.spacing_from_previous_metres;
		}

		element.length_metres = corrected_length(raw_length, input.boom_correction_metres);
		if (element.length_metres <= 0.0)
			return invalid_result(QStringLiteral("Boom correction makes an element length zero or negative."));

		element.half_length_metres = element.length_metres / 2.0;
		element.position_from_reflector_metres = position;
		result.elements.append(element);
	}

	result.boom_length_metres = position;
	result.assumptions << QStringLiteral("Yagi dimensions are empirical starting values, not an optimised antenna model.");
	result.assumptions << QStringLiteral("Element diameter, boom effects, spacing, matching, construction material, height, mounting, and surroundings alter final tuning.");
	result.assumptions << QStringLiteral("No NEC, Method-of-Moments, gain, front-to-back, SWR, or impedance model is used in this pass.");
	result.construction_notes << QStringLiteral("Build elements slightly long where practical and trim while measuring.");
	result.construction_notes << QStringLiteral("Use a balun or choke appropriate to the feed arrangement.");
	result.construction_notes << QStringLiteral("Mounting boom and element clamps can affect tuning.");
	result.tuning_notes << QStringLiteral("Check SWR with an analyser or suitable meter at low power first.");
	result.tuning_notes << QStringLiteral("Driven element feed and matching method are not designed in this pass.");

	if (input.preset == YagiPreset::Compact)
		result.assumptions << QStringLiteral("Compact Yagis are more sensitive to tuning and matching.");
	if (input.preset == YagiPreset::LongBoom)
		result.assumptions << QStringLiteral("Longer boom layouts may improve directivity, but real optimisation requires modelling.");
	if (input.boom_correction_metres > 0.0)
		result.assumptions << QStringLiteral("Boom correction is construction-specific and must be verified.");
	if (result.boom_length_metres >= IMPRACTICAL_YAGI_BOOM_M)
		result.warnings << QStringLiteral("Calculated boom length is impractical for ordinary sites and needs engineered supports.");
	else if (result.boom_length_metres >= LARGE_YAGI_BOOM_M)
		result.warnings << QStringLiteral("Calculated boom length is a large structure and needs careful mechanical design.");
	if (input.element_diameter_metres <= 0.0)
		result.warnings << QStringLiteral("Element diameter is not supplied, so diameter and boom-effect corrections are not modelled.");

	return result;
}

QString
yagi_element_role_label(YagiElementRole role)
{
	switch (role) {
	case YagiElementRole::Reflector:
		return QStringLiteral("Reflector");
	case YagiElementRole::Driven:
		return QStringLiteral("Driven");
	case YagiElementRole::Director:
		return QStringLiteral("Director");
	}

	return QStringLiteral("Unknown");
}

QString
yagi_preset_key(YagiPreset preset)
{
	switch (preset) {
	case YagiPreset::Conservative:
		return QStringLiteral("conservative");
	case YagiPreset::Compact:
		return QStringLiteral("compact");
	case YagiPreset::LongBoom:
		return QStringLiteral("long_boom");
	}

	return QStringLiteral("conservative");
}

QString
yagi_preset_label(YagiPreset preset)
{
	switch (preset) {
	case YagiPreset::Conservative:
		return QStringLiteral("Conservative");
	case YagiPreset::Compact:
		return QStringLiteral("Compact");
	case YagiPreset::LongBoom:
		return QStringLiteral("Long boom");
	}

	return QStringLiteral("Conservative");
}

YagiPreset
yagi_preset_from_key(const QString &key)
{
	if (key == QStringLiteral("compact"))
		return YagiPreset::Compact;
	if (key == QStringLiteral("long_boom"))
		return YagiPreset::LongBoom;

	return YagiPreset::Conservative;
}

}
