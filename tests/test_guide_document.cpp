// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_guide_document.cpp

#include "guides/guide_document.h"

#include <cassert>

namespace {

qantcal::project::AntennaProject
sample_project()
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();
	project.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	project.preferred_length_unit = qantcal::calculators::LengthUnit::Centimetres;
	project.title = QStringLiteral("Portable doublet");
	project.velocity_factor = 0.95;

	for (int i = 0; i < 2; ++i) {
		qantcal::project::AntennaTarget target;
		target.band_name = i == 0 ? QStringLiteral("40m") : QStringLiteral("20m");
		target.enabled = true;
		target.frequency_mhz = i == 0 ? 7.1 : 14.2;
		project.targets.append(target);

		qantcal::project::AntennaElement element;
		element.frequency_mhz = target.frequency_mhz;
		element.label = target.band_name;
		element.length_metres = i == 0 ? 20.057 : 10.028;
		element.notes = QStringLiteral("Trim and measure.");
		element.role = QStringLiteral("calculated_element");
		project.elements.append(element);
	}

	return project;
}

void
test_assumptions_and_safety_notes()
{
	const qantcal::guides::GuideDocument document =
		qantcal::guides::create_project_guide_document(sample_project(), qantcal::calculators::LengthUnit::Metres);
	bool found_assumptions = false;
	bool found_safety = false;

	for (const qantcal::guides::GuideSection &section : document.sections) {
		found_assumptions = found_assumptions || section.title == QStringLiteral("Assumptions and limitations");
		found_safety = found_safety || section.title == QStringLiteral("Safety and licence reminder");
	}

	assert(found_assumptions);
	assert(found_safety);
}

void
test_document_from_project()
{
	const qantcal::guides::GuideDocument document =
		qantcal::guides::create_project_guide_document(sample_project(), qantcal::calculators::LengthUnit::Metres);

	assert(document.title == QStringLiteral("Portable doublet"));
	assert(document.project_title == QStringLiteral("Portable doublet"));
	assert(document.antenna_type == QStringLiteral("Half-wave dipole"));
}

void
test_multi_band_rows()
{
	const qantcal::guides::GuideDocument document =
		qantcal::guides::create_project_guide_document(sample_project(), qantcal::calculators::LengthUnit::Metres);
	bool found_dimensions = false;

	for (const qantcal::guides::GuideSection &section : document.sections) {
		if (section.title == QStringLiteral("Antenna dimensions")) {
			found_dimensions = true;
			assert(section.table_rows.size() == 2);
		}
	}

	assert(found_dimensions);
}

void
test_target_band_frequency_data()
{
	const qantcal::guides::GuideDocument document =
		qantcal::guides::create_project_guide_document(sample_project(), qantcal::calculators::LengthUnit::Metres);
	bool found_targets = false;

	for (const qantcal::guides::GuideSection &section : document.sections) {
		if (section.title == QStringLiteral("Target bands and design frequencies")) {
			found_targets = true;
			assert(section.table_rows[0].cells[0] == QStringLiteral("40m"));
			assert(section.table_rows[0].cells[1].contains(QStringLiteral("7.100")));
		}
	}

	assert(found_targets);
}

void
test_unit_formatting()
{
	const qantcal::guides::GuideDocument document =
		qantcal::guides::create_project_guide_document(sample_project(), qantcal::calculators::LengthUnit::Centimetres);

	assert(document.dimensions_text.contains(QStringLiteral("2005.70 cm")));
}

}

int
main()
{
	test_assumptions_and_safety_notes();
	test_document_from_project();
	test_multi_band_rows();
	test_target_band_frequency_data();
	test_unit_formatting();

	return 0;
}
