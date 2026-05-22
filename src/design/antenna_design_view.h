// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_view.h

#ifndef QANTCAL_DESIGN_ANTENNA_DESIGN_VIEW_H
#define QANTCAL_DESIGN_ANTENNA_DESIGN_VIEW_H

#include <QGraphicsView>

namespace qantcal::design {

class AntennaDesignView : public QGraphicsView {
public:
	explicit AntennaDesignView(QGraphicsScene *scene, QWidget *parent = nullptr);

	void fit_design();
	void reset_zoom();
	void set_pan_mode(bool enabled);
	void zoom_in();
	void zoom_out();

protected:
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	bool middle_button_panning = false;
	bool pan_mode_enabled = false;
	QPoint last_pan_pos;
};

}

#endif
