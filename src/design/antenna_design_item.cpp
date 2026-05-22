// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_item.cpp

#include "antenna_design_item.h"

#include <QBrush>
#include <QPainterPath>
#include <QPen>

namespace qantcal::design {

namespace {

QPainterPath
path_from_descriptor(const project::DiagramItemDescriptor &descriptor)
{
	QPainterPath path;

	if (descriptor.kind == QStringLiteral("feedpoint")) {
		path.addEllipse(QPointF(0.0, 0.0), 8.0, 8.0);
		return path;
	}
	if (descriptor.kind == QStringLiteral("label")) {
		path.addRect(QRectF(0.0, 0.0, 1.0, 1.0));
		return path;
	}
	if (descriptor.kind == QStringLiteral("loop")) {
		if (!descriptor.points.isEmpty()) {
			path.moveTo(descriptor.points.first());
			for (int i = 1; i < descriptor.points.size(); ++i)
				path.lineTo(descriptor.points[i]);
			path.closeSubpath();
		}
		return path;
	}

	if (!descriptor.points.isEmpty()) {
		path.moveTo(descriptor.points.first());
		for (int i = 1; i < descriptor.points.size(); ++i)
			path.lineTo(descriptor.points[i]);
	}

	return path;
}

}

AntennaDesignItem::AntennaDesignItem(const project::DiagramItemDescriptor &descriptor, QGraphicsItem *parent)
	: QGraphicsPathItem(parent),
	  item_descriptor(descriptor)
{
	setPath(path_from_descriptor(descriptor));
	setPos(descriptor.position);
	setData(0, descriptor.id);
	setData(1, descriptor.kind);
	setData(2, descriptor.label);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable, !descriptor.locked);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setPen(QPen(QColor(30, 90, 150), descriptor.kind == QStringLiteral("feedpoint") ? 2.0 : 4.0));
	setBrush(descriptor.kind == QStringLiteral("feedpoint") ? QBrush(QColor(210, 80, 50)) : Qt::NoBrush);
}

project::DiagramItemDescriptor
AntennaDesignItem::descriptor() const
{
	project::DiagramItemDescriptor descriptor = item_descriptor;

	descriptor.position = pos();
	descriptor.label = item_label();

	return descriptor;
}

QString
AntennaDesignItem::item_id() const
{
	return item_descriptor.id;
}

QString
AntennaDesignItem::item_kind() const
{
	return item_descriptor.kind;
}

QString
AntennaDesignItem::item_label() const
{
	return data(2).toString();
}

void
AntennaDesignItem::set_item_label(const QString &label)
{
	item_descriptor.label = label;
	setData(2, label);
}

}
