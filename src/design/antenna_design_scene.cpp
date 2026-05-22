// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_scene.cpp

#include "antenna_design_scene.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsTextItem>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>

namespace qantcal::design {

namespace {

QString
length_label(const QString &prefix, double metres, calculators::LengthUnit length_unit)
{
	if (metres <= 0.0)
		return prefix;

	return QStringLiteral("%1: %2")
		.arg(prefix)
		.arg(QString::fromStdString(calculators::format_length(metres, length_unit)));
}

}

AntennaDesignScene::AntennaDesignScene(QObject *parent)
	: QGraphicsScene(parent)
{
	setSceneRect(-280.0, -180.0, 560.0, 360.0);

	calculators::AntennaCalculationResult result;
	result.ok = true;
	result.antenna_type = calculators::AntennaType::HalfWaveDipole;
	show_antenna_diagram(result, calculators::LengthUnit::Metres);
}

void
AntennaDesignScene::show_antenna_diagram(
	const calculators::AntennaCalculationResult &result,
	calculators::LengthUnit length_unit
)
{
	clear();

	const QPen wire_pen(QColor(30, 90, 150), 4.0);
	const QPen feed_pen(QColor(170, 60, 40), 3.0);
	const QPen guide_pen(QColor(120, 120, 120), 1.0, Qt::DashLine);
	const QBrush feed_brush(QColor(210, 80, 50));
	const QBrush tuner_brush(QColor(80, 130, 90));
	const QFont label_font(QStringLiteral("Sans Serif"), 10);
	const QString title_text = QString::fromUtf8(calculators::antenna_type_label(result.antenna_type));

	QGraphicsTextItem *title = addText(title_text, label_font);
	title->setDefaultTextColor(QColor(35, 35, 35));
	title->setPos(-250.0, -150.0);

	switch (result.antenna_type) {
	case calculators::AntennaType::HalfWaveDipole:
		addLine(-230.0, 0.0, -12.0, 0.0, wire_pen);
		addLine(12.0, 0.0, 230.0, 0.0, wire_pen);
		addEllipse(-12.0, -12.0, 24.0, 24.0, feed_pen, feed_brush);
		addLine(0.0, 12.0, 0.0, 95.0, feed_pen);
		addText(length_label(QStringLiteral("left leg"), result.leg_length_m, length_unit), label_font)->setPos(-220.0, -40.0);
		addText(length_label(QStringLiteral("right leg"), result.leg_length_m, length_unit), label_font)->setPos(80.0, -40.0);
		addText(QStringLiteral("centre feed"), label_font)->setPos(18.0, 18.0);
		break;
	case calculators::AntennaType::InvertedVee:
		addLine(0.0, -95.0, -220.0, 55.0, wire_pen);
		addLine(0.0, -95.0, 220.0, 55.0, wire_pen);
		addEllipse(-12.0, -107.0, 24.0, 24.0, feed_pen, feed_brush);
		addLine(-240.0, 75.0, 240.0, 75.0, guide_pen);
		addText(length_label(QStringLiteral("leg"), result.leg_length_m, length_unit), label_font)->setPos(-210.0, -25.0);
		addText(length_label(QStringLiteral("leg"), result.leg_length_m, length_unit), label_font)->setPos(105.0, -25.0);
		addText(QStringLiteral("apex feed"), label_font)->setPos(18.0, -120.0);
		break;
	case calculators::AntennaType::QuarterWaveVertical:
		addLine(0.0, 80.0, 0.0, -115.0, wire_pen);
		addEllipse(-12.0, 68.0, 24.0, 24.0, feed_pen, feed_brush);
		addLine(-230.0, 95.0, 230.0, 95.0, guide_pen);
		addLine(0.0, 85.0, -160.0, 130.0, wire_pen);
		addLine(0.0, 85.0, 160.0, 130.0, wire_pen);
		addLine(0.0, 85.0, -110.0, 95.0, wire_pen);
		addLine(0.0, 85.0, 110.0, 95.0, wire_pen);
		addText(length_label(QStringLiteral("radiator"), result.radiator_length_m, length_unit), label_font)->setPos(20.0, -35.0);
		addText(QStringLiteral("radials / counterpoise"), label_font)->setPos(-95.0, 130.0);
		break;
	case calculators::AntennaType::EndFedHalfWave:
		addLine(-220.0, 0.0, 230.0, 0.0, wire_pen);
		addRect(-245.0, -18.0, 28.0, 36.0, feed_pen, feed_brush);
		addLine(-231.0, 18.0, -231.0, 95.0, feed_pen);
		addText(length_label(QStringLiteral("wire"), result.total_length_m, length_unit), label_font)->setPos(-75.0, -40.0);
		addText(QStringLiteral("end feed / matcher"), label_font)->setPos(-250.0, 25.0);
		break;
	case calculators::AntennaType::FullWaveLoop: {
		QPolygonF loop;
		loop << QPointF(0.0, -115.0) << QPointF(210.0, 0.0) << QPointF(0.0, 115.0) << QPointF(-210.0, 0.0) << QPointF(0.0, -115.0);
		addPolygon(loop, wire_pen);
		addEllipse(-12.0, 103.0, 24.0, 24.0, feed_pen, feed_brush);
		addText(length_label(QStringLiteral("circumference"), result.total_length_m, length_unit), label_font)->setPos(-95.0, -145.0);
		addText(QStringLiteral("feed point"), label_font)->setPos(18.0, 110.0);
		break;
	}
	case calculators::AntennaType::RandomWire: {
		QPainterPath wire;
		wire.moveTo(-230.0, 20.0);
		wire.lineTo(-110.0, -45.0);
		wire.lineTo(30.0, 10.0);
		wire.lineTo(145.0, -65.0);
		wire.lineTo(230.0, -25.0);
		addPath(wire, wire_pen);
		addRect(-255.0, 2.0, 34.0, 36.0, feed_pen, tuner_brush);
		addLine(-238.0, 38.0, -238.0, 105.0, feed_pen);
		addText(QStringLiteral("tuner / matcher"), label_font)->setPos(-255.0, 45.0);
		addText(QStringLiteral("counterpoise required"), label_font)->setPos(-65.0, 85.0);
		break;
	}
	}

	if (!result.ok) {
		QGraphicsTextItem *error = addText(QString::fromStdString(result.error), label_font);
		error->setDefaultTextColor(QColor(150, 40, 35));
		error->setPos(-250.0, 135.0);
	}
}

void
AntennaDesignScene::show_project_diagram(
	const project::AntennaProject &project,
	calculators::LengthUnit length_unit
)
{
	if (project.elements.isEmpty()) {
		calculators::AntennaCalculationResult result;
		result.ok = true;
		result.antenna_type = project.antenna_type;
		show_antenna_diagram(result, length_unit);
		return;
	}

	clear();
	setSceneRect(-280.0, -180.0, 560.0, 360.0);

	const QPen wire_pen(QColor(30, 90, 150), 4.0);
	const QPen feed_pen(QColor(170, 60, 40), 2.0);
	const QBrush feed_brush(QColor(210, 80, 50));
	const QFont label_font(QStringLiteral("Sans Serif"), 9);
	double y = -130.0;

	QGraphicsTextItem *title = addText(project.title, label_font);
	title->setDefaultTextColor(QColor(35, 35, 35));
	title->setPos(-250.0, -165.0);

	for (const project::AntennaElement &element : project.elements) {
		const QString label = QStringLiteral("%1 MHz %2: %3")
			.arg(element.frequency_mhz, 0, 'f', 3)
			.arg(element.label)
			.arg(QString::fromStdString(calculators::format_length(element.length_metres, length_unit)));

		addLine(-220.0, y, 220.0, y, wire_pen);
		addEllipse(-8.0, y - 8.0, 16.0, 16.0, feed_pen, feed_brush);
		addText(label, label_font)->setPos(-220.0, y + 10.0);
		y += 58.0;

		if (y > 135.0)
			break;
	}
}

}
