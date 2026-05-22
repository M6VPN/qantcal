// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_item.h

#ifndef QANTCAL_DESIGN_ANTENNA_DESIGN_ITEM_H
#define QANTCAL_DESIGN_ANTENNA_DESIGN_ITEM_H

#include "project/antenna_project.h"

#include <QGraphicsPathItem>
#include <QString>

namespace qantcal::design {

class AntennaDesignItem : public QGraphicsPathItem {
public:
	explicit AntennaDesignItem(const project::DiagramItemDescriptor &descriptor, QGraphicsItem *parent = nullptr);

	project::DiagramItemDescriptor descriptor() const;
	QString item_id() const;
	QString item_kind() const;
	QString item_label() const;
	void set_item_label(const QString &label);

private:
	project::DiagramItemDescriptor item_descriptor;
};

}

#endif
