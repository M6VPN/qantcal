// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_document.cpp

#include "guide_document.h"

#include "calculators/rf_units.h"
#include "reference/band_reference.h"
#include "reference/mode_reference.h"
#include "reference/propagation_notes.h"
#include "reference/reach_estimator.h"

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

void
append_band_reference_section(GuideDocument &document, const reference::BandReference &band)
{
	document.sections.append(make_text_section(
		QStringLiteral("Band reference"),
		QStringLiteral("Band: %1\nService: %2\nFrequency range: %3-%4 MHz\nDesign frequency: %5 MHz\nMode notes: %6\nPropagation notes: %7\nWarning: %8")
			.arg(band.name)
			.arg(reference::band_service_label(band.service))
			.arg(band.lower_frequency_mhz, 0, 'f', 3)
			.arg(band.upper_frequency_mhz, 0, 'f', 3)
			.arg(band.design_frequency_mhz, 0, 'f', 3)
			.arg(band.mode_notes)
			.arg(band.propagation_notes)
			.arg(band.warning),
		band.service != reference::BandService::Amateur
	));
	if (band.service != reference::BandService::Amateur) {
		document.sections.append(make_text_section(
			QStringLiteral("Broadcast/reference warning"),
			QStringLiteral("These dimensions are suitable as starting points for receive antennas or legally authorised transmission only.\nqantcal does not grant authority to transmit."),
			true
		));
	}
}

void
append_propagation_sections(GuideDocument &document, const project::AntennaProject &project, calculators::LengthUnit length_unit)
{
	if (!project.propagation_settings.enabled || !project.propagation_settings.include_in_guides)
		return;

	reference::BandReference band;
	reference::ModeReference mode;
	reference::PropagationProfile profile;
	const double frequency_mhz = project.targets.isEmpty()
		? project.yagi_design.frequency_mhz
		: project.targets[0].frequency_mhz;
	QString band_name;

	if (reference::band_reference_by_frequency(frequency_mhz, band))
		band_name = band.name;
	if (reference::mode_reference_by_key(reference::mode_type_key(project.propagation_settings.mode), mode)) {
		document.sections.append(make_text_section(
			QStringLiteral("Mode reference"),
			QStringLiteral("%1\nBandwidth category: %2\nWeak-signal note: %3\nCaution: %4")
				.arg(mode.name)
				.arg(mode.bandwidth_category)
				.arg(mode.weak_signal_notes)
				.arg(mode.caution)
		));
	}
	if (!band_name.isEmpty() && reference::propagation_profile_by_band_name(band_name, profile)) {
		document.sections.append(make_text_section(
			QStringLiteral("Propagation notes"),
			QStringLiteral("%1\nCategories: %2\nDay/night tendency: %3\nCharacter: %4\nVariability: %5\n\n%6")
				.arg(band_name)
				.arg(profile.categories.join(QStringLiteral(", ")))
				.arg(profile.day_night_tendency)
				.arg(profile.character)
				.arg(profile.variability)
				.arg(reference::band_reference_warning()),
			true
		));
	}

	reference::ReachEstimateInput input;
	input.frequency_mhz = frequency_mhz;
	input.band_name = band_name;
	input.mode = project.propagation_settings.mode;
	input.environment = project.propagation_settings.environment;
	input.tx_height_metres = project.propagation_settings.tx_height_metres;
	input.rx_height_metres = project.propagation_settings.rx_height_metres;
	input.power_watts = project.propagation_settings.power_watts;
	input.has_power_watts = project.propagation_settings.has_power_watts;
	const reference::ReachEstimateResult estimate = reference::estimate_reach(input);
	if (estimate.ok) {
		QString body = estimate.summary;
		body += QStringLiteral("\nCategories: %1").arg(estimate.categories.join(QStringLiteral(", ")));
		if (estimate.includes_radio_horizon) {
			body += QStringLiteral("\nTX horizon: %1\nRX horizon: %2\nCombined radio horizon: %3")
				.arg(QString::fromStdString(calculators::format_length(estimate.tx_horizon_km * 1000.0, length_unit)))
				.arg(QString::fromStdString(calculators::format_length(estimate.rx_horizon_km * 1000.0, length_unit)))
				.arg(QString::fromStdString(calculators::format_length(estimate.combined_horizon_km * 1000.0, length_unit)));
		}
		body += QStringLiteral("\nWarnings:\n%1").arg(estimate.warnings.join(QStringLiteral("\n")));
		document.sections.append(make_text_section(QStringLiteral("Reach guidance"), body, true));
	}
}

void
append_lf_mf_section(GuideDocument &document, const project::AntennaProject &project, calculators::LengthUnit length_unit)
{
	if (!project.lf_mf_design.enabled)
		return;

	const project::LfMfProjectDesign &design = project.lf_mf_design;
	QString body;

	body += QStringLiteral("Band: %1\nService: %2\nCategory: %3\nFrequency: %4 kHz (%5 MHz)\nDesign type: %6\n")
		.arg(design.band_name)
		.arg(reference::band_service_label(design.band_service))
		.arg(design.category)
		.arg(design.frequency_mhz * 1000.0, 0, 'f', 3)
		.arg(design.frequency_mhz, 0, 'f', 6)
		.arg(calculators::lf_mf_design_type_label(design.design_type));
	body += QStringLiteral("Vertical height: %1\nHorizontal/top wire: %2\n")
		.arg(QString::fromStdString(calculators::format_length(design.vertical_height_metres, length_unit)))
		.arg(QString::fromStdString(calculators::format_length(design.horizontal_or_top_length_metres, length_unit)));
	if (design.has_estimated_capacitance)
		body += QStringLiteral("Estimated capacitance: %1 pF\n").arg(design.estimated_capacitance_pf, 0, 'f', 2);
	if (design.has_calculated_loading_inductance)
		body += QStringLiteral("Approximate loading inductance: %1 uH\n").arg(design.calculated_loading_inductance_uh, 0, 'f', 2);
	if (design.receive_only)
		body += QStringLiteral("Receive only: yes\n");
	body += QStringLiteral("\nLF/MF limitations: qantcal does not calculate efficiency, ERP/EIRP, field strength, radiation resistance, bandwidth, Q, or matching-network performance. Ground/counterpoise loss and loading-coil losses can dominate real systems.");
	body += QStringLiteral("\nSafety: high RF voltages can appear across loading coils and antenna ends. Loaded LF/MF antennas can have narrow bandwidth. Outdoor antennas require safe supports, weatherproofing, lightning/static precautions, and legal siting.");
	body += QStringLiteral("\nLegal: these dimensions are for receive antennas or legally authorised amateur, broadcast, or experimental use only. qantcal does not grant authority to transmit.");

	document.sections.append(make_text_section(QStringLiteral("LF/MF antenna guidance"), body, true));
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
	{
		reference::BandReference band;
		if (reference::band_reference_by_name(band_text, band))
			append_band_reference_section(document, band);
	}
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
	document.diagram_items = project.diagram_items;

	for (const project::AntennaElement &element : project.elements) {
		if (project.antenna_type == calculators::AntennaType::Yagi) {
			dimensions << QStringLiteral("%1 %2 MHz %3: %4")
				.arg(element.label)
				.arg(element.frequency_mhz, 0, 'f', 3)
				.arg(element.role)
				.arg(QString::fromStdString(calculators::format_length(element.length_metres, length_unit)));
		} else {
			dimensions << QStringLiteral("%1 %2 MHz: %3")
				.arg(element.label)
				.arg(element.frequency_mhz, 0, 'f', 3)
				.arg(QString::fromStdString(calculators::format_length(element.length_metres, length_unit)));
		}
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
		QStringLiteral("Project: %1\nAntenna: %2\nTargets: %3\nLength unit: %4\nVelocity / shortening factor: %5%6")
			.arg(document.project_title)
			.arg(document.antenna_type)
			.arg(project.targets.size())
			.arg(QString::fromStdString(calculators::length_unit_label(length_unit)))
			.arg(document.velocity_factor, 0, 'f', 3)
			.arg(project.antenna_type == calculators::AntennaType::Yagi
				? QStringLiteral("\nYagi elements: %1\nYagi preset: %2")
					.arg(project.yagi_design.element_count)
					.arg(calculators::yagi_preset_label(project.yagi_design.preset))
				: QString())
	));

	GuideSection targets_section;
	targets_section.title = QStringLiteral("Target bands and design frequencies");
	for (const project::AntennaTarget &target : project.targets) {
		if (!target.enabled)
			continue;
		GuideTableRow row;
		row.cells = QStringList{
			target.band_name,
			reference::band_service_label(target.band_service),
			QStringLiteral("%1 MHz").arg(target.frequency_mhz, 0, 'f', 3)
		};
		targets_section.table_rows.append(row);
		reference::BandReference band;
		if (reference::band_reference_by_name(target.band_name, band) && band.service != reference::BandService::Amateur)
			notes << band.warning;
	}
	document.sections.append(targets_section);
	for (const project::AntennaTarget &target : project.targets) {
		reference::BandReference band;
		if (target.enabled && reference::band_reference_by_name(target.band_name, band))
			append_band_reference_section(document, band);
	}
	document.sections.append(dimensions_section(dimensions));
	if (project.antenna_type == calculators::AntennaType::Yagi) {
		document.sections.append(make_text_section(
			QStringLiteral("Yagi construction and tuning notes"),
			QStringLiteral("Dimensions are starting points. Build elements slightly long where practical and trim while measuring. Driven element feed and matching method is not designed in this pass. Use a balun or choke appropriate to the feed arrangement. Check SWR with an analyser or suitable meter at low power first. Mounting boom and element clamps can affect tuning."),
			true
		));
	}
	document.sections.append(make_text_section(QStringLiteral("Diagram"), QStringLiteral("Diagram snapshot is rendered from the current design canvas.")));
	append_lf_mf_section(document, project, length_unit);
	append_propagation_sections(document, project, length_unit);
	append_standard_sections(document);

	return document;
}

}
