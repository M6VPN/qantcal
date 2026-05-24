// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/multi_band_guidance.h

#ifndef QANTCAL_PROJECT_MULTI_BAND_GUIDANCE_H
#define QANTCAL_PROJECT_MULTI_BAND_GUIDANCE_H

#include "antenna_project.h"

#include <QStringList>

namespace qantcal::project {

struct MultiBandGuidance {
	QStringList notes;
	QStringList warnings;
};

MultiBandGuidance create_multi_band_guidance(const AntennaProject &project);
QString multi_band_guidance_text(const MultiBandGuidance &guidance);

}

#endif
