// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/band_reference.h

#ifndef QANTCAL_REFERENCE_BAND_REFERENCE_H
#define QANTCAL_REFERENCE_BAND_REFERENCE_H

#include <QString>
#include <QVector>

namespace qantcal::reference {

enum class BandService {
	Amateur,
	Broadcast,
	Informal,
	Utility,
	Unknown
};

struct BandReference {
	QString name;
	BandService service = BandService::Unknown;
	double lower_frequency_mhz = 0.0;
	double upper_frequency_mhz = 0.0;
	double design_frequency_mhz = 0.0;
	QString wavelength_label;
	QString category;
	QString mode_notes;
	QString antenna_notes;
	QString propagation_notes;
	QString use_case_notes;
	QString warning;
};

enum class BandReferenceFilter {
	Amateur,
	BroadcastReference,
	All
};

const QVector<BandReference> &band_references();
QVector<BandReference> band_references(BandReferenceFilter filter);
bool band_reference_by_frequency(double frequency_mhz, BandReference &reference);
bool band_reference_by_name(const QString &name, BandReference &reference);
QString band_reference_warning();
QString band_service_key(BandService service);
QString band_service_label(BandService service);
BandService band_service_from_key(const QString &key);

}

#endif
