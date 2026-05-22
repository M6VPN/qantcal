// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_scene.h

#ifndef QANTCAL_DESIGN_ANTENNA_DESIGN_SCENE_H
#define QANTCAL_DESIGN_ANTENNA_DESIGN_SCENE_H

#include "calculators/antenna_calculator.h"
#include "calculators/rf_units.h"
#include "calculators/yagi_calculator.h"
#include "project/antenna_project.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QPointF>

#include <functional>

class QUndoStack;

namespace qantcal::design {

class AntennaDesignScene : public QGraphicsScene {
public:
	explicit AntennaDesignScene(QObject *parent = nullptr);

	void show_antenna_diagram(
		const calculators::AntennaCalculationResult &result,
		calculators::LengthUnit length_unit
	);
	void show_project_diagram(
		const project::AntennaProject &project,
		calculators::LengthUnit length_unit
	);
	void show_yagi_diagram(
		const calculators::YagiDesignResult &result,
		calculators::LengthUnit length_unit
	);
	void render_to_painter(QPainter &painter, const QRectF &target_rect);
	void set_item_moved_callback(std::function<void(const project::DiagramItemDescriptor &)> callback);
	void set_undo_stack(QUndoStack *stack);

protected:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
	std::function<void(const project::DiagramItemDescriptor &)> item_moved_callback;
	QGraphicsItem *moving_item = nullptr;
	QPointF move_start_pos;
	QUndoStack *undo_stack = nullptr;
};

}

#endif
