// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/yagi_calculator.h

#ifndef QANTCAL_CALCULATORS_YAGI_CALCULATOR_H
#define QANTCAL_CALCULATORS_YAGI_CALCULATOR_H

#include "rf_units.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace qantcal::calculators {

enum class YagiElementRole {
	Reflector,
	Driven,
	Director
};

enum class YagiPreset {
	Conservative,
	Compact,
	LongBoom
};

struct YagiDesignInput {
	double frequency_mhz = 0.0;
	int element_count = 3;
	double element_shortening_factor = 0.95;
	double boom_correction_metres = 0.0;
	double element_diameter_metres = 0.010;
	YagiPreset preset = YagiPreset::Conservative;
	LengthUnit preferred_length_unit = LengthUnit::Metres;
};

struct YagiElement {
	QString label;
	YagiElementRole role = YagiElementRole::Director;
	double length_metres = 0.0;
	double half_length_metres = 0.0;
	double position_from_reflector_metres = 0.0;
	double spacing_from_previous_metres = 0.0;
	QString notes;
};

struct YagiDesignResult {
	bool ok = false;
	QString error_message;
	double frequency_mhz = 0.0;
	double wavelength_metres = 0.0;
	double boom_length_metres = 0.0;
	QVector<YagiElement> elements;
	QStringList assumptions;
	QStringList construction_notes;
	QStringList tuning_notes;
	QStringList warnings;
};

YagiDesignResult calculate_yagi(const YagiDesignInput &input);
QString yagi_element_role_label(YagiElementRole role);
QString yagi_preset_key(YagiPreset preset);
QString yagi_preset_label(YagiPreset preset);
YagiPreset yagi_preset_from_key(const QString &key);

}

#endif
