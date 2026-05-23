// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/reach_estimator.h

#ifndef QANTCAL_REFERENCE_REACH_ESTIMATOR_H
#define QANTCAL_REFERENCE_REACH_ESTIMATOR_H

#include "mode_reference.h"

#include <QString>
#include <QStringList>

namespace qantcal::reference {

enum class EnvironmentProfile {
	Indoor,
	Urban,
	Suburban,
	Rural,
	HilltopOpen
};

struct ReachEstimateInput {
	double frequency_mhz = 0.0;
	QString band_name;
	ModeType mode = ModeType::SsbVoice;
	double tx_height_metres = 0.0;
	double rx_height_metres = 0.0;
	bool has_power_watts = false;
	double power_watts = 0.0;
	EnvironmentProfile environment = EnvironmentProfile::Suburban;
	bool notes_only = false;
};

struct ReachEstimateResult {
	bool ok = false;
	QString error_message;
	QString summary;
	QStringList categories;
	QStringList warnings;
	double tx_horizon_km = 0.0;
	double rx_horizon_km = 0.0;
	double combined_horizon_km = 0.0;
	bool includes_radio_horizon = false;
};

ReachEstimateResult estimate_reach(const ReachEstimateInput &input);
QString environment_profile_key(EnvironmentProfile environment);
QString environment_profile_label(EnvironmentProfile environment);
EnvironmentProfile environment_profile_from_key(const QString &key);

}

#endif
