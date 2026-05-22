// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/project_serializer.cpp

#include "project_serializer.h"

#include <QJsonArray>

namespace qantcal::project {

namespace {

bool
is_valid_factor(double factor)
{
	return factor >= calculators::MIN_WIRE_FACTOR && factor <= calculators::MAX_WIRE_FACTOR;
}

bool
is_valid_yagi_factor(double factor)
{
	return factor >= 0.85 && factor <= 1.00;
}

QJsonObject
diagram_item_to_json(const DiagramItemDescriptor &item)
{
	QJsonArray points;
	QJsonObject object;

	for (const QPointF &point : item.points) {
		QJsonObject point_object;
		point_object.insert(QStringLiteral("x"), point.x());
		point_object.insert(QStringLiteral("y"), point.y());
		points.append(point_object);
	}

	object.insert(QStringLiteral("kind"), item.kind);
	object.insert(QStringLiteral("id"), item.id);
	object.insert(QStringLiteral("label"), item.label);
	object.insert(QStringLiteral("length_metres"), item.length_metres);
	object.insert(QStringLiteral("locked"), item.locked);
	object.insert(QStringLiteral("points"), points);
	object.insert(QStringLiteral("position_x"), item.position.x());
	object.insert(QStringLiteral("position_y"), item.position.y());

	return object;
}

QJsonObject
yagi_design_to_json(const YagiProjectDesign &design)
{
	QJsonObject object;

	object.insert(QStringLiteral("boomCorrectionMetres"), design.boom_correction_metres);
	object.insert(QStringLiteral("elementCount"), design.element_count);
	object.insert(QStringLiteral("elementDiameterMetres"), design.element_diameter_metres);
	object.insert(QStringLiteral("elementShorteningFactor"), design.element_shortening_factor);
	object.insert(QStringLiteral("frequencyMhz"), design.frequency_mhz);
	object.insert(QStringLiteral("preset"), calculators::yagi_preset_key(design.preset));

	return object;
}

QJsonObject
element_to_json(const AntennaElement &element)
{
	QJsonObject object;

	object.insert(QStringLiteral("frequency_mhz"), element.frequency_mhz);
	object.insert(QStringLiteral("label"), element.label);
	object.insert(QStringLiteral("length_metres"), element.length_metres);
	object.insert(QStringLiteral("notes"), element.notes);
	object.insert(QStringLiteral("role"), element.role);

	return object;
}

QJsonObject
target_to_json(const AntennaTarget &target)
{
	QJsonObject object;

	object.insert(QStringLiteral("band_name"), target.band_name);
	object.insert(QStringLiteral("enabled"), target.enabled);
	object.insert(QStringLiteral("frequency_mhz"), target.frequency_mhz);

	return object;
}

bool
read_diagram_items(const QJsonArray &array, QVector<DiagramItemDescriptor> &items, QString &error_message)
{
	for (const QJsonValue &value : array) {
		const QJsonObject object = value.toObject();
		const double length_metres = object.value(QStringLiteral("length_metres")).toDouble(0.0);
		DiagramItemDescriptor item;

		if (length_metres < 0.0) {
			error_message = QStringLiteral("Diagram item length cannot be negative.");
			return false;
		}

		item.kind = object.value(QStringLiteral("kind")).toString();
		item.id = object.value(QStringLiteral("id")).toString();
		item.label = object.value(QStringLiteral("label")).toString();
		item.length_metres = length_metres;
		item.locked = object.value(QStringLiteral("locked")).toBool(false);
		item.position = QPointF(
			object.value(QStringLiteral("position_x")).toDouble(0.0),
			object.value(QStringLiteral("position_y")).toDouble(0.0)
		);

		for (const QJsonValue &point_value : object.value(QStringLiteral("points")).toArray()) {
			const QJsonObject point_object = point_value.toObject();
			item.points.append(QPointF(
				point_object.value(QStringLiteral("x")).toDouble(),
				point_object.value(QStringLiteral("y")).toDouble()
			));
		}

		items.append(item);
	}

	return true;
}

bool
read_elements(const QJsonArray &array, QVector<AntennaElement> &elements, QString &error_message)
{
	for (const QJsonValue &value : array) {
		const QJsonObject object = value.toObject();
		AntennaElement element;

		element.frequency_mhz = object.value(QStringLiteral("frequency_mhz")).toDouble();
		element.label = object.value(QStringLiteral("label")).toString();
		element.length_metres = object.value(QStringLiteral("length_metres")).toDouble();
		element.notes = object.value(QStringLiteral("notes")).toString();
		element.role = object.value(QStringLiteral("role")).toString();

		if (element.frequency_mhz < 0.0) {
			error_message = QStringLiteral("Element frequency cannot be negative.");
			return false;
		}
		if (element.length_metres < 0.0) {
			error_message = QStringLiteral("Element length cannot be negative.");
			return false;
		}

		elements.append(element);
	}

	return true;
}

bool
read_targets(const QJsonArray &array, QVector<AntennaTarget> &targets, QString &error_message)
{
	for (const QJsonValue &value : array) {
		const QJsonObject object = value.toObject();
		AntennaTarget target;

		target.band_name = object.value(QStringLiteral("band_name")).toString();
		target.enabled = object.value(QStringLiteral("enabled")).toBool(true);
		target.frequency_mhz = object.value(QStringLiteral("frequency_mhz")).toDouble();

		if (target.frequency_mhz < 0.0) {
			error_message = QStringLiteral("Target frequency cannot be negative.");
			return false;
		}

		targets.append(target);
	}

	return true;
}

bool
read_yagi_design(const QJsonObject &object, YagiProjectDesign &design, QString &error_message)
{
	if (!object.contains(QStringLiteral("yagiDesign")))
		return true;

	const QJsonObject yagi_object = object.value(QStringLiteral("yagiDesign")).toObject();
	const int element_count = yagi_object.value(QStringLiteral("elementCount")).toInt(3);
	const double element_shortening_factor = yagi_object.value(QStringLiteral("elementShorteningFactor")).toDouble(0.95);
	const double element_diameter_metres = yagi_object.value(QStringLiteral("elementDiameterMetres")).toDouble(0.010);
	const double frequency_mhz = yagi_object.value(QStringLiteral("frequencyMhz")).toDouble(0.0);
	const double boom_correction_metres = yagi_object.value(QStringLiteral("boomCorrectionMetres")).toDouble(0.0);

	if (element_count < 2 || element_count > 10) {
		error_message = QStringLiteral("Yagi element count must be between 2 and 10.");
		return false;
	}
	if (!is_valid_yagi_factor(element_shortening_factor)) {
		error_message = QStringLiteral("Yagi element shortening factor must be between 0.85 and 1.00.");
		return false;
	}
	if (element_diameter_metres < 0.0) {
		error_message = QStringLiteral("Yagi element diameter cannot be negative.");
		return false;
	}
	if (boom_correction_metres < 0.0) {
		error_message = QStringLiteral("Yagi boom correction cannot be negative.");
		return false;
	}
	if (frequency_mhz < 0.0) {
		error_message = QStringLiteral("Yagi frequency cannot be negative.");
		return false;
	}

	design.enabled = true;
	design.element_count = element_count;
	design.element_shortening_factor = element_shortening_factor;
	design.element_diameter_metres = element_diameter_metres;
	design.frequency_mhz = frequency_mhz;
	design.boom_correction_metres = boom_correction_metres;
	design.preset = calculators::yagi_preset_from_key(yagi_object.value(QStringLiteral("preset")).toString());

	return true;
}

}

bool
from_json(const QJsonObject &object, AntennaProject &project, QString &error_message)
{
	const int schema_version = object.value(QStringLiteral("schema_version")).toInt(-1);
	const double velocity_factor = object.value(QStringLiteral("velocity_factor")).toDouble(calculators::DEFAULT_WIRE_FACTOR);

	if (schema_version != CURRENT_SCHEMA_VERSION) {
		error_message = QStringLiteral("Unsupported or missing schema_version.");
		return false;
	}
	if (!is_valid_factor(velocity_factor)) {
		error_message = QStringLiteral("Velocity factor must be between 0.50 and 1.00.");
		return false;
	}

	AntennaProject parsed;
	parsed.schema_version = schema_version;
	parsed.antenna_type = antenna_type_from_key(object.value(QStringLiteral("antenna_type")).toString());
	parsed.created_utc = object.value(QStringLiteral("created_utc")).toString();
	parsed.notes = object.value(QStringLiteral("notes")).toString();
	parsed.preferred_length_unit = calculators::length_unit_from_key(object.value(QStringLiteral("preferred_length_unit")).toString().toStdString());
	parsed.title = object.value(QStringLiteral("title")).toString(QStringLiteral("Untitled Project"));
	parsed.updated_utc = object.value(QStringLiteral("updated_utc")).toString();
	parsed.velocity_factor = velocity_factor;

	if (!read_targets(object.value(QStringLiteral("targets")).toArray(), parsed.targets, error_message))
		return false;
	if (!read_elements(object.value(QStringLiteral("elements")).toArray(), parsed.elements, error_message))
		return false;
	if (!read_diagram_items(object.value(QStringLiteral("diagram_items")).toArray(), parsed.diagram_items, error_message))
		return false;
	if (!read_yagi_design(object, parsed.yagi_design, error_message))
		return false;

	project = parsed;

	return true;
}

QJsonObject
to_json(const AntennaProject &project)
{
	QJsonArray diagram_items;
	QJsonArray elements;
	QJsonArray targets;
	QJsonObject object;

	for (const AntennaTarget &target : project.targets)
		targets.append(target_to_json(target));
	for (const AntennaElement &element : project.elements)
		elements.append(element_to_json(element));
	for (const DiagramItemDescriptor &item : project.diagram_items)
		diagram_items.append(diagram_item_to_json(item));

	object.insert(QStringLiteral("schema_version"), project.schema_version);
	object.insert(QStringLiteral("antenna_type"), antenna_type_to_key(project.antenna_type));
	object.insert(QStringLiteral("created_utc"), project.created_utc);
	object.insert(QStringLiteral("diagram_items"), diagram_items);
	object.insert(QStringLiteral("elements"), elements);
	object.insert(QStringLiteral("notes"), project.notes);
	object.insert(QStringLiteral("preferred_length_unit"), QString::fromStdString(calculators::length_unit_key(project.preferred_length_unit)));
	object.insert(QStringLiteral("targets"), targets);
	object.insert(QStringLiteral("title"), project.title);
	object.insert(QStringLiteral("updated_utc"), project.updated_utc);
	object.insert(QStringLiteral("velocity_factor"), project.velocity_factor);
	if (project.yagi_design.enabled)
		object.insert(QStringLiteral("yagiDesign"), yagi_design_to_json(project.yagi_design));

	return object;
}

}
