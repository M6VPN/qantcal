// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/antenna_project.h

#ifndef QANTCAL_PROJECT_ANTENNA_PROJECT_H
#define QANTCAL_PROJECT_ANTENNA_PROJECT_H

#include "calculators/antenna_calculator.h"
#include "calculators/rf_units.h"
#include "calculators/yagi_calculator.h"
#include "reference/band_reference.h"
#include "reference/mode_reference.h"
#include "reference/reach_estimator.h"

#include <QPointF>
#include <QString>
#include <QVector>

namespace qantcal::project {

constexpr int CURRENT_SCHEMA_VERSION = 1;

struct AntennaTarget {
	QString band_name;
	reference::BandService band_service = reference::BandService::Unknown;
	bool enabled = true;
	double frequency_mhz = 0.0;
};

struct AntennaElement {
	QString label;
	QString notes;
	QString role;
	double frequency_mhz = 0.0;
	double length_metres = 0.0;
};

struct DiagramItemDescriptor {
	QString id;
	QString kind;
	QString label;
	QVector<QPointF> points;
	double length_metres = 0.0;
	QPointF position;
	bool locked = false;
};

struct YagiProjectDesign {
	bool enabled = false;
	int element_count = 3;
	calculators::YagiPreset preset = calculators::YagiPreset::Conservative;
	double frequency_mhz = 0.0;
	double element_shortening_factor = 0.95;
	double element_diameter_metres = 0.010;
	double boom_correction_metres = 0.0;
};

struct PropagationProjectSettings {
	bool enabled = false;
	bool include_in_guides = true;
	reference::ModeType mode = reference::ModeType::SsbVoice;
	reference::EnvironmentProfile environment = reference::EnvironmentProfile::Suburban;
	double tx_height_metres = 10.0;
	double rx_height_metres = 10.0;
	double power_watts = 0.0;
	bool has_power_watts = false;
};

struct AntennaProject {
	QString created_utc;
	QString notes;
	QString title = "Untitled Project";
	QString updated_utc;
	QVector<AntennaElement> elements;
	QVector<AntennaTarget> targets;
	QVector<DiagramItemDescriptor> diagram_items;
	YagiProjectDesign yagi_design;
	PropagationProjectSettings propagation_settings;
	calculators::AntennaType antenna_type = calculators::AntennaType::HalfWaveDipole;
	calculators::LengthUnit preferred_length_unit = calculators::LengthUnit::Metres;
	double velocity_factor = calculators::DEFAULT_WIRE_FACTOR;
	int schema_version = CURRENT_SCHEMA_VERSION;
};

AntennaProject default_project();
QString antenna_type_to_key(calculators::AntennaType antenna_type);
calculators::AntennaType antenna_type_from_key(const QString &key);

}

#endif
