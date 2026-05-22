// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/design_commands.h

#ifndef QANTCAL_DESIGN_DESIGN_COMMANDS_H
#define QANTCAL_DESIGN_DESIGN_COMMANDS_H

#include <QGraphicsItem>
#include <QPointF>
#include <QUndoCommand>

#include <functional>

namespace qantcal::design {

class MoveItemCommand : public QUndoCommand {
public:
	MoveItemCommand(QGraphicsItem *item, const QPointF &old_pos, const QPointF &new_pos, std::function<void()> callback = {});

	void redo() override;
	void undo() override;

private:
	void apply_position(const QPointF &position);

	std::function<void()> moved_callback;
	QGraphicsItem *target_item = nullptr;
	QPointF new_position;
	QPointF old_position;
};

}

#endif
