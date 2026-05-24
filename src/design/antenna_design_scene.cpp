// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_scene.cpp

#include "antenna_design_scene.h"

#include "antenna_design_item.h"
#include "design_commands.h"

#include <QBrush>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QFont>
#include <QGraphicsTextItem>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QUndoStack>

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

void
add_label_for_item(QGraphicsItem *parent, const QString &label, double y_offset)
{
	QGraphicsTextItem *text = new QGraphicsTextItem(label, parent);

	text->setDefaultTextColor(QColor(45, 45, 45));
	text->setPos(0.0, y_offset);
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
		addLine(-230.0, 0.0, -12.0, 0.0, wire_pen)->setFlag(QGraphicsItem::ItemIsSelectable, true);
		addLine(12.0, 0.0, 230.0, 0.0, wire_pen)->setFlag(QGraphicsItem::ItemIsSelectable, true);
		addEllipse(-12.0, -12.0, 24.0, 24.0, feed_pen, feed_brush);
		addLine(0.0, 12.0, 0.0, 95.0, feed_pen);
		addText(length_label(QStringLiteral("left leg"), result.leg_length_m, length_unit), label_font)->setPos(-220.0, -40.0);
		addText(length_label(QStringLiteral("right leg"), result.leg_length_m, length_unit), label_font)->setPos(80.0, -40.0);
		addText(QStringLiteral("centre feed"), label_font)->setPos(18.0, 18.0);
		break;
	case calculators::AntennaType::FoldedDipole: {
		QPainterPath folded;
		folded.moveTo(-230.0, -22.0);
		folded.lineTo(230.0, -22.0);
		folded.lineTo(230.0, 22.0);
		folded.lineTo(14.0, 22.0);
		folded.moveTo(-14.0, 22.0);
		folded.lineTo(-230.0, 22.0);
		folded.lineTo(-230.0, -22.0);
		addPath(folded, wire_pen)->setFlag(QGraphicsItem::ItemIsSelectable, true);
		addEllipse(-12.0, 10.0, 24.0, 24.0, feed_pen, feed_brush);
		addLine(0.0, 34.0, 0.0, 105.0, feed_pen);
		addText(length_label(QStringLiteral("conductor span"), result.total_length_m, length_unit), label_font)->setPos(-210.0, -62.0);
		addText(QStringLiteral("folded return conductor"), label_font)->setPos(60.0, 34.0);
		addText(QStringLiteral("balanced centre feed"), label_font)->setPos(18.0, 58.0);
		break;
	}
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
	case calculators::AntennaType::Yagi:
		addLine(-230.0, 0.0, 230.0, 0.0, guide_pen);
		addLine(-170.0, -85.0, -170.0, 85.0, wire_pen);
		addLine(0.0, -75.0, 0.0, 75.0, wire_pen);
		addLine(140.0, -65.0, 140.0, 65.0, wire_pen);
		addEllipse(-12.0, -12.0, 24.0, 24.0, feed_pen, feed_brush);
		addText(QStringLiteral("Yagi starting layout"), label_font)->setPos(-85.0, -130.0);
		addText(QStringLiteral("forward direction ->"), label_font)->setPos(85.0, 30.0);
		break;
	}

	if (!result.ok) {
		QGraphicsTextItem *error = addText(QString::fromStdString(result.error), label_font);
		error->setDefaultTextColor(QColor(150, 40, 35));
		error->setPos(-250.0, 135.0);
	}
}

void
AntennaDesignScene::show_yagi_diagram(
	const calculators::YagiDesignResult &result,
	calculators::LengthUnit length_unit
)
{
	clear();
	setSceneRect(-320.0, -210.0, 640.0, 420.0);

	const QPen boom_pen(QColor(90, 90, 90), 3.0);
	const QPen wire_pen(QColor(30, 90, 150), 4.0);
	const QPen feed_pen(QColor(170, 60, 40), 3.0);
	const QBrush feed_brush(QColor(210, 80, 50));
	const QFont label_font(QStringLiteral("Sans Serif"), 8);
	const double left = -250.0;
	const double right = 250.0;
	const double boom_y = 0.0;
	const double scale = result.boom_length_metres > 0.0
		? (right - left) / result.boom_length_metres
		: 1.0;

	QGraphicsTextItem *title = addText(QStringLiteral("Yagi top view"), label_font);
	title->setDefaultTextColor(QColor(35, 35, 35));
	title->setPos(-290.0, -185.0);

	if (!result.ok || result.elements.isEmpty()) {
		QGraphicsTextItem *error = addText(result.error_message.isEmpty() ? QStringLiteral("No Yagi diagram available") : result.error_message, label_font);
		error->setDefaultTextColor(QColor(150, 40, 35));
		error->setPos(-250.0, -20.0);
		return;
	}

	addLine(left, boom_y, right, boom_y, boom_pen);
	addText(
		length_label(QStringLiteral("boom"), result.boom_length_metres, length_unit),
		label_font
	)->setPos(left, 24.0);
	addLine(right - 45.0, -38.0, right, -38.0, feed_pen);
	addLine(right, -38.0, right - 13.0, -45.0, feed_pen);
	addLine(right, -38.0, right - 13.0, -31.0, feed_pen);
	addText(QStringLiteral("forward direction"), label_font)->setPos(right - 135.0, -64.0);

	for (int i = 0; i < result.elements.size(); ++i) {
		const calculators::YagiElement &element = result.elements[i];
		const double x = left + (element.position_from_reflector_metres * scale);
		const double half_pixels = 45.0 + (element.length_metres / result.elements[0].length_metres * 55.0);
		const double label_y = i % 2 == 0 ? half_pixels + 8.0 : -half_pixels - 42.0;
		QGraphicsLineItem *line = addLine(x, -half_pixels, x, half_pixels, wire_pen);
		line->setFlag(QGraphicsItem::ItemIsSelectable, true);
		if (element.role == calculators::YagiElementRole::Driven)
			addEllipse(x - 9.0, -9.0, 18.0, 18.0, feed_pen, feed_brush);
		addText(
			QStringLiteral("%1\n%2")
				.arg(element.label)
				.arg(QString::fromStdString(calculators::format_length(element.length_metres, length_unit))),
			label_font
		)->setPos(x - 34.0, label_y);
	}
}

void
AntennaDesignScene::show_lf_mf_diagram(
	const calculators::LfMfAntennaResult &result,
	calculators::LfMfDesignType design_type,
	calculators::LengthUnit length_unit
)
{
	clear();
	setSceneRect(-280.0, -180.0, 560.0, 360.0);

	const QPen wire_pen(QColor(30, 90, 150), 4.0);
	const QPen feed_pen(QColor(170, 60, 40), 3.0);
	const QPen guide_pen(QColor(120, 120, 120), 1.0, Qt::DashLine);
	const QBrush coil_brush(QColor(230, 190, 80));
	const QBrush feed_brush(QColor(210, 80, 50));
	const QFont label_font(QStringLiteral("Sans Serif"), 9);

	addText(QStringLiteral("LF/MF antenna guide"), label_font)->setPos(-250.0, -155.0);
	if (!result.ok) {
		QGraphicsTextItem *error = addText(result.error_message, label_font);
		error->setDefaultTextColor(QColor(150, 40, 35));
		error->setPos(-230.0, -20.0);
		return;
	}

	switch (design_type) {
	case calculators::LfMfDesignType::FullSizeReference:
		addLine(-230.0, 0.0, 230.0, 0.0, wire_pen);
		addText(QStringLiteral("full-size reference, not to scale"), label_font)->setPos(-95.0, -45.0);
		addText(length_label(QStringLiteral("quarter wave"), result.quarter_wave_metres, length_unit), label_font)->setPos(-230.0, 40.0);
		break;
	case calculators::LfMfDesignType::ShortLoadedVertical:
		addLine(0.0, 80.0, 0.0, -115.0, wire_pen);
		addEllipse(-18.0, 42.0, 36.0, 28.0, feed_pen, coil_brush);
		addLine(-210.0, 95.0, 210.0, 95.0, guide_pen);
		addLine(0.0, 85.0, -150.0, 125.0, wire_pen);
		addLine(0.0, 85.0, 150.0, 125.0, wire_pen);
		addText(QStringLiteral("base loading coil"), label_font)->setPos(25.0, 38.0);
		addText(length_label(QStringLiteral("vertical"), result.total_wire_length_metres, length_unit), label_font)->setPos(25.0, -70.0);
		break;
	case calculators::LfMfDesignType::InvertedL:
		addLine(-120.0, 90.0, -120.0, -95.0, wire_pen);
		addLine(-120.0, -95.0, 210.0, -95.0, wire_pen);
		addRect(-145.0, 55.0, 50.0, 35.0, feed_pen, feed_brush);
		addText(QStringLiteral("loading / matcher"), label_font)->setPos(-90.0, 54.0);
		addText(length_label(QStringLiteral("wire"), result.total_wire_length_metres, length_unit), label_font)->setPos(-30.0, -130.0);
		break;
	case calculators::LfMfDesignType::TopLoadedT:
		addLine(0.0, 90.0, 0.0, -95.0, wire_pen);
		addLine(-180.0, -95.0, 180.0, -95.0, wire_pen);
		addEllipse(-18.0, 42.0, 36.0, 28.0, feed_pen, coil_brush);
		addText(QStringLiteral("top loading"), label_font)->setPos(45.0, -125.0);
		addText(QStringLiteral("base loading / tuning"), label_font)->setPos(25.0, 45.0);
		break;
	case calculators::LfMfDesignType::ReceiveOnlyCompact:
		addEllipse(-90.0, -80.0, 180.0, 160.0, wire_pen);
		addRect(120.0, -25.0, 45.0, 50.0, feed_pen, feed_brush);
		addLine(0.0, 0.0, 120.0, 0.0, feed_pen);
		addText(QStringLiteral("receive-only loop / active probe placeholder"), label_font)->setPos(-170.0, 95.0);
		break;
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
		const int element_index = &element - project.elements.constData();
		project::DiagramItemDescriptor descriptor = element_index < project.diagram_items.size()
			? project.diagram_items[element_index]
			: project::DiagramItemDescriptor();
		if (descriptor.id.isEmpty())
			descriptor.id = QStringLiteral("element-%1").arg(element.frequency_mhz, 0, 'f', 3);
		if (descriptor.kind.isEmpty())
			descriptor.kind = QStringLiteral("line");
		if (descriptor.label.isEmpty())
			descriptor.label = element.label;
		if (descriptor.length_metres <= 0.0)
			descriptor.length_metres = element.length_metres;
		if (descriptor.position.isNull())
			descriptor.position = QPointF(0.0, y);
		if (descriptor.points.isEmpty())
			descriptor.points = { QPointF(-220.0, 0.0), QPointF(220.0, 0.0) };
		const QString label = QStringLiteral("%1 MHz %2: %3")
			.arg(element.frequency_mhz, 0, 'f', 3)
			.arg(element.label)
			.arg(QString::fromStdString(calculators::format_length(element.length_metres, length_unit)));
		AntennaDesignItem *item = new AntennaDesignItem(descriptor);
		const QPointF feed_position = descriptor.position;

		addItem(item);
		if (descriptor.kind != QStringLiteral("yagi_element"))
			addEllipse(feed_position.x() - 8.0, feed_position.y() - 8.0, 16.0, 16.0, feed_pen, feed_brush)->setFlag(QGraphicsItem::ItemIsSelectable, true);
		add_label_for_item(item, label, 10.0);
		y += 58.0;

		if (y > 135.0)
			break;
	}
}

void
AntennaDesignScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	AntennaDesignItem *item = dynamic_cast<AntennaDesignItem *>(itemAt(event->scenePos(), QTransform()));

	if (item == nullptr) {
		QGraphicsScene::contextMenuEvent(event);
		return;
	}

	QMenu menu;
	QAction *rename_action = menu.addAction(QStringLiteral("Rename label"));
	QAction *reset_action = menu.addAction(QStringLiteral("Reset position"));
	QAction *selected = menu.exec(event->screenPos());

	if (selected == rename_action) {
		bool ok = false;
		const QString label = QInputDialog::getText(nullptr, QStringLiteral("Rename label"), QStringLiteral("Label"), QLineEdit::Normal, item->item_label(), &ok);
		if (ok) {
			item->set_item_label(label);
			if (item_moved_callback)
				item_moved_callback(item->descriptor());
		}
		return;
	}
	if (selected == reset_action) {
		const QPointF old_pos = item->pos();
		const QPointF new_pos(0.0, old_pos.y());
		if (undo_stack != nullptr)
			undo_stack->push(new MoveItemCommand(item, old_pos, new_pos, [this, item]() { if (item_moved_callback) item_moved_callback(item->descriptor()); }));
		else
			item->setPos(new_pos);
	}
}

void
AntennaDesignScene::render_to_painter(QPainter &painter, const QRectF &target_rect)
{
	if (!painter.isActive())
		return;

	if (items().isEmpty()) {
		painter.save();
		painter.setPen(QPen(QColor(120, 120, 120), 1.5));
		painter.drawRect(target_rect);
		painter.drawText(target_rect.adjusted(10.0, 10.0, -10.0, -10.0), Qt::AlignLeft | Qt::AlignTop, QStringLiteral("No diagram available"));
		painter.restore();
		return;
	}

	render(&painter, target_rect, itemsBoundingRect(), Qt::KeepAspectRatio);
}

void
AntennaDesignScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	moving_item = itemAt(event->scenePos(), QTransform());
	move_start_pos = moving_item != nullptr ? moving_item->pos() : QPointF();

	QGraphicsScene::mousePressEvent(event);
}

void
AntennaDesignScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseReleaseEvent(event);

	if (moving_item == nullptr)
		return;

	const QPointF end_pos = moving_item->pos();
	AntennaDesignItem *design_item = dynamic_cast<AntennaDesignItem *>(moving_item);

	if (design_item != nullptr && end_pos != move_start_pos) {
		if (undo_stack != nullptr) {
			moving_item->setPos(move_start_pos);
			undo_stack->push(new MoveItemCommand(design_item, move_start_pos, end_pos, [this, design_item]() { if (item_moved_callback) item_moved_callback(design_item->descriptor()); }));
		} else if (item_moved_callback) {
			item_moved_callback(design_item->descriptor());
		}
	}

	moving_item = nullptr;
}

void
AntennaDesignScene::set_item_moved_callback(std::function<void(const project::DiagramItemDescriptor &)> callback)
{
	item_moved_callback = callback;
}

void
AntennaDesignScene::set_undo_stack(QUndoStack *stack)
{
	undo_stack = stack;
}

}
