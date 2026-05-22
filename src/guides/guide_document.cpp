// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_document.cpp

#include "guide_document.h"

#include "calculators/rf_units.h"

namespace qantcal::guides {

namespace {

void
append_dimension(QStringList &lines, const QString &label, double metres, calculators::LengthUnit length_unit)
{
	if (metres <= 0.0)
		return;

	lines << QStringLiteral("%1: %2")
		.arg(label)
		.arg(QString::fromStdString(calculators::format_length(metres, length_unit)));
}

}

GuideDocument
create_guide_document(
	const calculators::AntennaCalculationResult &result,
	calculators::LengthUnit length_unit,
	const QString &band_text
)
{
	QStringList dimensions;
	QStringList notes;
	GuideDocument document;

	document.antenna_type = QString::fromUtf8(calculators::antenna_type_label(result.antenna_type));
	document.band_text = band_text;
	document.frequency_text = QString::fromStdString(calculators::format_mhz(result.frequency_mhz));
	document.length_unit = length_unit;
	document.title = QStringLiteral("qantcal antenna guide");

	append_dimension(dimensions, QStringLiteral("Wavelength"), result.wavelength_m, length_unit);
	append_dimension(dimensions, QStringLiteral("Total length"), result.total_length_m, length_unit);
	append_dimension(dimensions, QStringLiteral("Per-leg length"), result.leg_length_m, length_unit);
	append_dimension(dimensions, QStringLiteral("Radiator length"), result.radiator_length_m, length_unit);

	notes << QString::fromStdString(result.counterpoise_note);
	notes << QString::fromStdString(result.matching_note);
	notes << QString::fromStdString(result.trimming_note);

	document.dimensions_text = dimensions.join(QStringLiteral("\n"));
	document.notes_text = notes.join(QStringLiteral("\n"));

	return document;
}

}
