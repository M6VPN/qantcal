// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_project_serializer.cpp

#include "project/project_serializer.h"

#include <cassert>

namespace {

qantcal::project::AntennaProject
sample_project(int target_count)
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();

	project.title = QStringLiteral("Portable dipole");
	project.notes = QStringLiteral("Field day notes");
	project.preferred_length_unit = qantcal::calculators::LengthUnit::Centimetres;
	project.velocity_factor = 0.95;

	for (int i = 0; i < target_count; ++i) {
		qantcal::project::AntennaTarget target;
		target.band_name = QStringLiteral("%1m").arg(40 / (i + 1));
		target.enabled = true;
		target.frequency_mhz = 7.1 + i;
		project.targets.append(target);
	}

	qantcal::project::AntennaElement element;
	element.frequency_mhz = 7.1;
	element.label = QStringLiteral("40m dipole total");
	element.length_metres = 20.057;
	element.role = QStringLiteral("leg");
	project.elements.append(element);

	return project;
}

void
test_default_round_trip()
{
	const qantcal::project::AntennaProject project = qantcal::project::default_project();
	qantcal::project::AntennaProject parsed;
	QString error;

	assert(qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(parsed.schema_version == qantcal::project::CURRENT_SCHEMA_VERSION);
	assert(parsed.title == "Untitled Project");
}

void
test_length_unit_round_trip()
{
	const qantcal::project::AntennaProject project = sample_project(1);
	qantcal::project::AntennaProject parsed;
	QString error;

	assert(qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(parsed.preferred_length_unit == qantcal::calculators::LengthUnit::Centimetres);
}

void
test_multiple_targets_round_trip()
{
	const qantcal::project::AntennaProject project = sample_project(3);
	qantcal::project::AntennaProject parsed;
	QString error;

	assert(qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(parsed.targets.size() == 3);
	assert(parsed.elements.size() == 1);
}

void
test_negative_frequency_rejected()
{
	qantcal::project::AntennaProject project = sample_project(1);
	qantcal::project::AntennaProject parsed;
	QString error;

	project.targets[0].frequency_mhz = -7.1;

	assert(!qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(!error.isEmpty());
}

void
test_negative_length_rejected()
{
	qantcal::project::AntennaProject project = sample_project(1);
	qantcal::project::AntennaProject parsed;
	QString error;

	project.elements[0].length_metres = -20.0;

	assert(!qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(!error.isEmpty());
}

void
test_one_target_round_trip()
{
	const qantcal::project::AntennaProject project = sample_project(1);
	qantcal::project::AntennaProject parsed;
	QString error;

	assert(qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(parsed.targets.size() == 1);
	assert(parsed.targets[0].frequency_mhz == 7.1);
}

void
test_unknown_field_ignored()
{
	const qantcal::project::AntennaProject project = sample_project(1);
	QJsonObject object = qantcal::project::to_json(project);
	qantcal::project::AntennaProject parsed;
	QString error;

	object.insert(QStringLiteral("future_field"), QStringLiteral("ignored"));

	assert(qantcal::project::from_json(object, parsed, error));
	assert(parsed.title == project.title);
}

void
test_unsupported_schema_rejected()
{
	QJsonObject object = qantcal::project::to_json(sample_project(1));
	qantcal::project::AntennaProject parsed;
	QString error;

	object.insert(QStringLiteral("schema_version"), 999);

	assert(!qantcal::project::from_json(object, parsed, error));
	assert(!error.isEmpty());
}

}

int
main()
{
	test_default_round_trip();
	test_length_unit_round_trip();
	test_multiple_targets_round_trip();
	test_negative_frequency_rejected();
	test_negative_length_rejected();
	test_one_target_round_trip();
	test_unknown_field_ignored();
	test_unsupported_schema_rejected();

	return 0;
}
