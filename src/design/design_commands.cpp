// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/design_commands.cpp

#include "design_commands.h"

namespace qantcal::design {

MoveItemCommand::MoveItemCommand(QGraphicsItem *item, const QPointF &old_pos, const QPointF &new_pos, std::function<void()> callback)
	: moved_callback(callback),
	  target_item(item),
	  new_position(new_pos),
	  old_position(old_pos)
{
	setText(QStringLiteral("Move design item"));
}

void
MoveItemCommand::apply_position(const QPointF &position)
{
	if (target_item != nullptr)
		target_item->setPos(position);
	if (moved_callback)
		moved_callback();
}

void
MoveItemCommand::redo()
{
	apply_position(new_position);
}

void
MoveItemCommand::undo()
{
	apply_position(old_position);
}

}
