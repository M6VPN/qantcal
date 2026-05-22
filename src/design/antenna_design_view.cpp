// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_view.cpp

#include "antenna_design_view.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>

namespace qantcal::design {

namespace {

constexpr double ZOOM_FACTOR = 1.2;

}

AntennaDesignView::AntennaDesignView(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent)
{
	setRenderHint(QPainter::Antialiasing);
	setDragMode(QGraphicsView::RubberBandDrag);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void
AntennaDesignView::fit_design()
{
	if (scene() != nullptr)
		fitInView(scene()->itemsBoundingRect().adjusted(-20.0, -20.0, 20.0, 20.0), Qt::KeepAspectRatio);
}

void
AntennaDesignView::reset_zoom()
{
	resetTransform();
}

void
AntennaDesignView::set_pan_mode(bool enabled)
{
	pan_mode_enabled = enabled;
	setDragMode(enabled ? QGraphicsView::ScrollHandDrag : QGraphicsView::RubberBandDrag);
}

void
AntennaDesignView::zoom_in()
{
	scale(ZOOM_FACTOR, ZOOM_FACTOR);
}

void
AntennaDesignView::zoom_out()
{
	scale(1.0 / ZOOM_FACTOR, 1.0 / ZOOM_FACTOR);
}

void
AntennaDesignView::mouseMoveEvent(QMouseEvent *event)
{
	if (middle_button_panning) {
		const QPoint delta = event->pos() - last_pan_pos;
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
		verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
		last_pan_pos = event->pos();
		event->accept();
		return;
	}

	QGraphicsView::mouseMoveEvent(event);
}

void
AntennaDesignView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton) {
		middle_button_panning = true;
		last_pan_pos = event->pos();
		setCursor(Qt::ClosedHandCursor);
		event->accept();
		return;
	}

	QGraphicsView::mousePressEvent(event);
}

void
AntennaDesignView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton) {
		middle_button_panning = false;
		setCursor(pan_mode_enabled ? Qt::OpenHandCursor : Qt::ArrowCursor);
		event->accept();
		return;
	}

	QGraphicsView::mouseReleaseEvent(event);
}

void
AntennaDesignView::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() > 0)
		zoom_in();
	else
		zoom_out();

	event->accept();
}

}
