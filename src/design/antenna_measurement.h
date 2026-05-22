// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_measurement.h

#ifndef QANTCAL_DESIGN_ANTENNA_MEASUREMENT_H
#define QANTCAL_DESIGN_ANTENNA_MEASUREMENT_H

#include "calculators/rf_units.h"

#include <QPointF>
#include <QString>

namespace qantcal::design {

double scene_distance(const QPointF &start, const QPointF &end);
double scaled_length_metres(const QPointF &start, const QPointF &end, double metres_per_scene_unit);
QString format_measurement(double metres, calculators::LengthUnit length_unit);

}

#endif
