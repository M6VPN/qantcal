// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_antenna_measurement.cpp

#include "design/antenna_measurement.h"
#include "design/design_commands.h"

#include <QGraphicsRectItem>
#include <QUndoStack>

#include <cassert>
#include <cmath>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_distance_between_points()
{
	assert(near_value(qantcal::design::scene_distance(QPointF(0.0, 0.0), QPointF(3.0, 4.0)), 5.0, 0.001));
}

void
test_format_centimetres()
{
	assert(qantcal::design::format_measurement(1.5, qantcal::calculators::LengthUnit::Centimetres) == QStringLiteral("150.00 cm"));
}

void
test_format_feet_inches()
{
	assert(qantcal::design::format_measurement(qantcal::calculators::feet_to_metres(10.5), qantcal::calculators::LengthUnit::FeetInches) == QStringLiteral("10 ft 6.0 in"));
}

void
test_format_metres()
{
	assert(qantcal::design::format_measurement(1.5, qantcal::calculators::LengthUnit::Metres) == QStringLiteral("1.500 m"));
}

void
test_move_command()
{
	QGraphicsRectItem item;
	QUndoStack stack;

	item.setPos(QPointF(0.0, 0.0));
	stack.push(new qantcal::design::MoveItemCommand(&item, QPointF(0.0, 0.0), QPointF(10.0, 5.0)));

	assert(item.pos() == QPointF(10.0, 5.0));
	stack.undo();
	assert(item.pos() == QPointF(0.0, 0.0));
	stack.redo();
	assert(item.pos() == QPointF(10.0, 5.0));
}

}

int
main()
{
	test_distance_between_points();
	test_format_centimetres();
	test_format_feet_inches();
	test_format_metres();
	test_move_command();

	return 0;
}
