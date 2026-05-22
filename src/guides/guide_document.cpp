// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_document.cpp

#include "guide_document.h"

#include "calculators/rf_units.h"

#include <QDateTime>

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

GuideSection
dimensions_section(const QStringList &dimensions)
{
	GuideSection section;
	section.title = QStringLiteral("Antenna dimensions");

	for (const QString &line : dimensions) {
		const QStringList parts = line.split(QStringLiteral(": "));
		GuideTableRow row;
		row.cells = parts.size() == 2 ? parts : QStringList{ line };
		section.table_rows.append(row);
	}

	return section;
}

void
append_standard_sections(GuideDocument &document)
{
	document.sections.append(make_text_section(
		QStringLiteral("Construction notes"),
		document.notes_text,
		false
	));
	document.sections.append(make_text_section(
		QStringLiteral("Assumptions and limitations"),
		QStringLiteral("Calculated dimensions are starting points only. Real installations vary due to height, ground, nearby metal, insulation, bends, wire diameter, baluns, matching networks, traps, radials, and surroundings. Trim and verify with an analyser, VNA, or SWR checks."),
		true
	));
	document.sections.append(make_text_section(
		QStringLiteral("Safety and licence reminder"),
		QStringLiteral("Users must obey their licence terms, band plans, RF exposure rules, local planning restrictions, and electrical safety requirements. This guide is not legal, safety, or compliance authority."),
		true
	));
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
	document.generated_utc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
	document.length_unit = length_unit;
	document.project_title = QStringLiteral("Current antenna");
	document.title = QStringLiteral("qantcal antenna guide");
	document.velocity_factor = result.shortening_factor;

	append_dimension(dimensions, QStringLiteral("Wavelength"), result.wavelength_m, length_unit);
	append_dimension(dimensions, QStringLiteral("Total length"), result.total_length_m, length_unit);
	append_dimension(dimensions, QStringLiteral("Per-leg length"), result.leg_length_m, length_unit);
	append_dimension(dimensions, QStringLiteral("Radiator length"), result.radiator_length_m, length_unit);

	notes << QString::fromStdString(result.counterpoise_note);
	notes << QString::fromStdString(result.matching_note);
	notes << QString::fromStdString(result.trimming_note);

	document.dimensions_text = dimensions.join(QStringLiteral("\n"));
	document.notes_text = notes.join(QStringLiteral("\n"));
	document.sections.append(make_text_section(
		QStringLiteral("Title / project summary"),
		QStringLiteral("Project: %1\nAntenna: %2\nBand: %3\nDesign frequency: %4\nLength unit: %5\nVelocity / shortening factor: %6")
			.arg(document.project_title)
			.arg(document.antenna_type)
			.arg(document.band_text)
			.arg(document.frequency_text)
			.arg(QString::fromStdString(calculators::length_unit_label(length_unit)))
			.arg(document.velocity_factor, 0, 'f', 3)
	));
	document.sections.append(make_text_section(
		QStringLiteral("Target bands and design frequencies"),
		QStringLiteral("%1 - %2").arg(document.band_text).arg(document.frequency_text)
	));
	document.sections.append(dimensions_section(dimensions));
	document.sections.append(make_text_section(QStringLiteral("Diagram"), QStringLiteral("Diagram snapshot is rendered from the current design canvas.")));
	append_standard_sections(document);

	return document;
}

GuideDocument
create_project_guide_document(
	const project::AntennaProject &project,
	calculators::LengthUnit length_unit
)
{
	QStringList dimensions;
	QStringList notes;
	GuideDocument document;

	document.antenna_type = QString::fromUtf8(calculators::antenna_type_label(project.antenna_type));
	document.band_text = QStringLiteral("Project targets");
	document.frequency_text = QStringLiteral("multiple targets");
	document.generated_utc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
	document.length_unit = length_unit;
	document.project_notes = project.notes;
	document.project_title = project.title;
	document.title = project.title.isEmpty() ? QStringLiteral("qantcal antenna guide") : project.title;
	document.velocity_factor = project.velocity_factor;

	for (const project::AntennaElement &element : project.elements) {
		dimensions << QStringLiteral("%1 %2 MHz: %3")
			.arg(element.label)
			.arg(element.frequency_mhz, 0, 'f', 3)
			.arg(QString::fromStdString(calculators::format_length(element.length_metres, length_unit)));
		if (!element.notes.isEmpty())
			notes << element.notes;
	}

	if (!project.notes.isEmpty())
		notes << project.notes;
	notes << QStringLiteral("Multi-band physical interaction is future work. Current targets are calculated independently.");

	document.dimensions_text = dimensions.join(QStringLiteral("\n"));
	document.notes_text = notes.join(QStringLiteral("\n"));
	document.sections.append(make_text_section(
		QStringLiteral("Title / project summary"),
		QStringLiteral("Project: %1\nAntenna: %2\nTargets: %3\nLength unit: %4\nVelocity / shortening factor: %5")
			.arg(document.project_title)
			.arg(document.antenna_type)
			.arg(project.targets.size())
			.arg(QString::fromStdString(calculators::length_unit_label(length_unit)))
			.arg(document.velocity_factor, 0, 'f', 3)
	));

	GuideSection targets_section;
	targets_section.title = QStringLiteral("Target bands and design frequencies");
	for (const project::AntennaTarget &target : project.targets) {
		if (!target.enabled)
			continue;
		GuideTableRow row;
		row.cells = QStringList{
			target.band_name,
			QStringLiteral("%1 MHz").arg(target.frequency_mhz, 0, 'f', 3)
		};
		targets_section.table_rows.append(row);
	}
	document.sections.append(targets_section);
	document.sections.append(dimensions_section(dimensions));
	document.sections.append(make_text_section(QStringLiteral("Diagram"), QStringLiteral("Diagram snapshot is rendered from the current design canvas.")));
	append_standard_sections(document);

	return document;
}

}
