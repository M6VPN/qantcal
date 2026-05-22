// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_document.h

#ifndef QANTCAL_GUIDES_GUIDE_DOCUMENT_H
#define QANTCAL_GUIDES_GUIDE_DOCUMENT_H

#include "calculators/antenna_calculator.h"
#include "calculators/rf_units.h"
#include "project/antenna_project.h"

#include <QString>
#include <QStringList>

namespace qantcal::guides {

struct GuideDocument {
	QString antenna_type;
	QString band_text;
	QString dimensions_text;
	QString frequency_text;
	QString notes_text;
	QString title;
	calculators::LengthUnit length_unit = calculators::LengthUnit::Metres;
};

GuideDocument create_guide_document(
	const calculators::AntennaCalculationResult &result,
	calculators::LengthUnit length_unit,
	const QString &band_text
);
GuideDocument create_project_guide_document(
	const project::AntennaProject &project,
	calculators::LengthUnit length_unit
);

}

#endif
