// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/antenna_project.h

#ifndef QANTCAL_PROJECT_ANTENNA_PROJECT_H
#define QANTCAL_PROJECT_ANTENNA_PROJECT_H

#include "calculators/antenna_calculator.h"
#include "calculators/rf_units.h"

#include <QPointF>
#include <QString>
#include <QVector>

namespace qantcal::project {

constexpr int CURRENT_SCHEMA_VERSION = 1;

struct AntennaTarget {
	QString band_name;
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
	QString kind;
	QString label;
	QVector<QPointF> points;
	double length_metres = 0.0;
};

struct AntennaProject {
	QString created_utc;
	QString notes;
	QString title = "Untitled Project";
	QString updated_utc;
	QVector<AntennaElement> elements;
	QVector<AntennaTarget> targets;
	QVector<DiagramItemDescriptor> diagram_items;
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
