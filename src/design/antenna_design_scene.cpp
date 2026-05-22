// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_scene.cpp

#include "antenna_design_scene.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsTextItem>
#include <QPen>

namespace qantcal::design {

AntennaDesignScene::AntennaDesignScene(QObject *parent)
	: QGraphicsScene(parent)
{
	setSceneRect(-260.0, -160.0, 520.0, 320.0);
	show_placeholder_diagram(QStringLiteral("Half-wave dipole"));
}

void
AntennaDesignScene::show_placeholder_diagram(const QString &antenna_label)
{
	clear();

	const QPen wire_pen(QColor(30, 90, 150), 4.0);
	const QPen feed_pen(QColor(170, 60, 40), 3.0);
	const QPen guide_pen(QColor(120, 120, 120), 1.0, Qt::DashLine);
	const QBrush feed_brush(QColor(210, 80, 50));
	const QFont label_font(QStringLiteral("Sans Serif"), 10);

	addLine(-220.0, 0.0, -12.0, 0.0, wire_pen);
	addLine(12.0, 0.0, 220.0, 0.0, wire_pen);
	addEllipse(-12.0, -12.0, 24.0, 24.0, feed_pen, feed_brush);
	addLine(0.0, 12.0, 0.0, 96.0, feed_pen);
	addLine(-240.0, 96.0, 240.0, 96.0, guide_pen);

	QGraphicsTextItem *title = addText(antenna_label, label_font);
	title->setDefaultTextColor(QColor(35, 35, 35));
	title->setPos(-220.0, -120.0);

	QGraphicsTextItem *feed_label = addText(QStringLiteral("feed point"), label_font);
	feed_label->setDefaultTextColor(QColor(80, 80, 80));
	feed_label->setPos(18.0, 18.0);

	QGraphicsTextItem *note = addText(QStringLiteral("diagram placeholder"), label_font);
	note->setDefaultTextColor(QColor(95, 95, 95));
	note->setPos(-220.0, 110.0);
}

}
