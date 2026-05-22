// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_measurement.cpp

#include "antenna_measurement.h"

#include <cmath>

namespace qantcal::design {

double
scene_distance(const QPointF &start, const QPointF &end)
{
	const double dx = end.x() - start.x();
	const double dy = end.y() - start.y();

	return std::sqrt((dx * dx) + (dy * dy));
}

double
scaled_length_metres(const QPointF &start, const QPointF &end, double metres_per_scene_unit)
{
	if (metres_per_scene_unit <= 0.0)
		return 0.0;

	return scene_distance(start, end) * metres_per_scene_unit;
}

QString
format_measurement(double metres, calculators::LengthUnit length_unit)
{
	return QString::fromStdString(calculators::format_length(metres, length_unit));
}

}
