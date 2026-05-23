// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/propagation_notes.h

#ifndef QANTCAL_REFERENCE_PROPAGATION_NOTES_H
#define QANTCAL_REFERENCE_PROPAGATION_NOTES_H

#include <QString>
#include <QStringList>

namespace qantcal::reference {

struct PropagationProfile {
	QString band_name;
	QStringList categories;
	QString day_night_tendency;
	QString character;
	QString noise_tendency;
	QString antenna_practicality;
	QString variability;
};

bool propagation_profile_by_band_name(const QString &band_name, PropagationProfile &profile);
bool propagation_profile_by_frequency(double frequency_mhz, PropagationProfile &profile);

}

#endif
