// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/design/antenna_design_scene.h

#ifndef QANTCAL_DESIGN_ANTENNA_DESIGN_SCENE_H
#define QANTCAL_DESIGN_ANTENNA_DESIGN_SCENE_H

#include "calculators/antenna_calculator.h"

#include <QGraphicsScene>

namespace qantcal::design {

class AntennaDesignScene : public QGraphicsScene {
public:
	explicit AntennaDesignScene(QObject *parent = nullptr);

	void show_antenna_diagram(const calculators::AntennaCalculationResult &result);
};

}

#endif
