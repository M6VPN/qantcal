// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_project_serializer.cpp

#include "project/project_serializer.h"

#include <QJsonArray>

#include <cassert>
#include <cmath>

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

	qantcal::project::DiagramItemDescriptor item;
	item.id = QStringLiteral("diagram-1");
	item.kind = QStringLiteral("line");
	item.label = QStringLiteral("wire");
	item.length_metres = 20.057;
	item.locked = false;
	item.position = QPointF(10.0, 20.0);
	item.points.append(QPointF(-1.0, 0.0));
	item.points.append(QPointF(1.0, 0.0));
	project.diagram_items.append(item);

	return project;
}

bool
near(double actual, double expected)
{
	return std::fabs(actual - expected) <= 0.000001;
}

qantcal::project::AntennaProject
sample_yagi_project()
{
	qantcal::project::AntennaProject project = sample_project(1);

	project.antenna_type = qantcal::calculators::AntennaType::Yagi;
	project.yagi_design.enabled = true;
	project.yagi_design.element_count = 5;
	project.yagi_design.preset = qantcal::calculators::YagiPreset::Compact;
	project.yagi_design.frequency_mhz = 144.3;
	project.yagi_design.element_shortening_factor = 0.94;
	project.yagi_design.element_diameter_metres = 0.012;
	project.yagi_design.boom_correction_metres = 0.005;

	return project;
}

qantcal::project::AntennaProject
sample_propagation_project()
{
	qantcal::project::AntennaProject project = sample_project(1);

	project.propagation_settings.enabled = true;
	project.propagation_settings.include_in_guides = true;
	project.propagation_settings.mode = qantcal::reference::ModeType::Cw;
	project.propagation_settings.environment = qantcal::reference::EnvironmentProfile::Rural;
	project.propagation_settings.tx_height_metres = 12.0;
	project.propagation_settings.rx_height_metres = 8.0;
	project.propagation_settings.has_power_watts = true;
	project.propagation_settings.power_watts = 25.0;

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
test_missing_diagram_fields_defaults()
{
	QJsonObject object = qantcal::project::to_json(sample_project(1));
	QJsonArray diagram_items;
	QJsonObject diagram_item;
	qantcal::project::AntennaProject parsed;
	QString error;

	diagram_item.insert(QStringLiteral("kind"), QStringLiteral("line"));
	diagram_item.insert(QStringLiteral("label"), QStringLiteral("legacy wire"));
	diagram_items.append(diagram_item);
	object.insert(QStringLiteral("diagram_items"), diagram_items);

	assert(qantcal::project::from_json(object, parsed, error));
	assert(parsed.diagram_items.size() == 1);
	assert(parsed.diagram_items[0].position == QPointF(0.0, 0.0));
	assert(!parsed.diagram_items[0].locked);
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
test_propagation_settings_round_trip()
{
	const qantcal::project::AntennaProject project = sample_propagation_project();
	qantcal::project::AntennaProject parsed;
	QString error;

	assert(qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(parsed.propagation_settings.enabled);
	assert(parsed.propagation_settings.mode == qantcal::reference::ModeType::Cw);
	assert(parsed.propagation_settings.environment == qantcal::reference::EnvironmentProfile::Rural);
	assert(near(parsed.propagation_settings.tx_height_metres, 12.0));
	assert(near(parsed.propagation_settings.rx_height_metres, 8.0));
	assert(parsed.propagation_settings.has_power_watts);
	assert(near(parsed.propagation_settings.power_watts, 25.0));
}

void
test_round_trip_preserves_diagram_fields()
{
	const qantcal::project::AntennaProject project = sample_project(1);
	qantcal::project::AntennaProject parsed;
	QString error;

	assert(qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(parsed.diagram_items.size() == 1);
	assert(parsed.diagram_items[0].id == QStringLiteral("diagram-1"));
	assert(parsed.diagram_items[0].kind == QStringLiteral("line"));
	assert(parsed.diagram_items[0].label == QStringLiteral("wire"));
	assert(parsed.diagram_items[0].points.size() == 2);
	assert(parsed.diagram_items[0].position == QPointF(10.0, 20.0));
}

void
test_unknown_field_ignored()
{
	const qantcal::project::AntennaProject project = sample_project(1);
	QJsonObject object = qantcal::project::to_json(project);
	QJsonArray diagram_items = object.value(QStringLiteral("diagram_items")).toArray();
	QJsonObject diagram_item = diagram_items[0].toObject();
	qantcal::project::AntennaProject parsed;
	QString error;

	object.insert(QStringLiteral("future_field"), QStringLiteral("ignored"));
	diagram_item.insert(QStringLiteral("future_diagram_field"), QStringLiteral("ignored"));
	diagram_items.replace(0, diagram_item);
	object.insert(QStringLiteral("diagram_items"), diagram_items);

	assert(qantcal::project::from_json(object, parsed, error));
	assert(parsed.title == project.title);
}

void
test_older_project_without_yagi_design_loads()
{
	QJsonObject object = qantcal::project::to_json(sample_project(1));
	qantcal::project::AntennaProject parsed;
	QString error;

	object.remove(QStringLiteral("yagiDesign"));

	assert(qantcal::project::from_json(object, parsed, error));
	assert(!parsed.yagi_design.enabled);
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

void
test_yagi_design_round_trip()
{
	const qantcal::project::AntennaProject project = sample_yagi_project();
	qantcal::project::AntennaProject parsed;
	QString error;

	assert(qantcal::project::from_json(qantcal::project::to_json(project), parsed, error));
	assert(parsed.antenna_type == qantcal::calculators::AntennaType::Yagi);
	assert(parsed.yagi_design.enabled);
	assert(parsed.yagi_design.element_count == 5);
	assert(parsed.yagi_design.preset == qantcal::calculators::YagiPreset::Compact);
	assert(near(parsed.yagi_design.frequency_mhz, 144.3));
	assert(near(parsed.yagi_design.element_shortening_factor, 0.94));
	assert(near(parsed.yagi_design.element_diameter_metres, 0.012));
	assert(near(parsed.yagi_design.boom_correction_metres, 0.005));
}

void
test_yagi_design_missing_optional_fields_defaults()
{
	QJsonObject object = qantcal::project::to_json(sample_yagi_project());
	QJsonObject yagi = object.value(QStringLiteral("yagiDesign")).toObject();
	qantcal::project::AntennaProject parsed;
	QString error;

	yagi.remove(QStringLiteral("boomCorrectionMetres"));
	yagi.remove(QStringLiteral("elementDiameterMetres"));
	yagi.remove(QStringLiteral("frequencyMhz"));
	yagi.remove(QStringLiteral("preset"));
	object.insert(QStringLiteral("yagiDesign"), yagi);

	assert(qantcal::project::from_json(object, parsed, error));
	assert(parsed.yagi_design.enabled);
	assert(parsed.yagi_design.boom_correction_metres == 0.0);
	assert(parsed.yagi_design.element_diameter_metres == 0.010);
	assert(parsed.yagi_design.frequency_mhz == 0.0);
	assert(parsed.yagi_design.preset == qantcal::calculators::YagiPreset::Conservative);
}

}

int
main()
{
	test_default_round_trip();
	test_length_unit_round_trip();
	test_missing_diagram_fields_defaults();
	test_multiple_targets_round_trip();
	test_negative_frequency_rejected();
	test_negative_length_rejected();
	test_one_target_round_trip();
	test_older_project_without_yagi_design_loads();
	test_propagation_settings_round_trip();
	test_round_trip_preserves_diagram_fields();
	test_unknown_field_ignored();
	test_unsupported_schema_rejected();
	test_yagi_design_round_trip();
	test_yagi_design_missing_optional_fields_defaults();

	return 0;
}
