// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/band_reference.h

#ifndef QANTCAL_REFERENCE_BAND_REFERENCE_H
#define QANTCAL_REFERENCE_BAND_REFERENCE_H

#include <QString>
#include <QVector>

namespace qantcal::reference {

struct BandReference {
	QString name;
	double lower_frequency_mhz = 0.0;
	double upper_frequency_mhz = 0.0;
	double design_frequency_mhz = 0.0;
	QString wavelength_label;
	QString category;
	QString antenna_notes;
	QString propagation_notes;
	QString use_case_notes;
	QString warning;
};

const QVector<BandReference> &band_references();
bool band_reference_by_frequency(double frequency_mhz, BandReference &reference);
bool band_reference_by_name(const QString &name, BandReference &reference);
QString band_reference_warning();

}

#endif
