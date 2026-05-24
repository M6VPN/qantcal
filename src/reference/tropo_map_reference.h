// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/tropo_map_reference.h

#ifndef QANTCAL_REFERENCE_TROPO_MAP_REFERENCE_H
#define QANTCAL_REFERENCE_TROPO_MAP_REFERENCE_H

#include <QDate>
#include <QString>
#include <QUrl>
#include <QVector>

namespace qantcal::reference {

struct TropoMapLevel {
	int index = 0;
	QString colour_hex;
	QString label;
};

struct TropoMapRegion {
	QString code;
	QString label;
	QString page_path;
};

QString tropo_cache_file_name(const TropoMapRegion &region, int forecast_hour, const QDate &date);
QString tropo_cache_key(const QDate &date);
const QVector<int> &tropo_forecast_hours();
const QVector<TropoMapLevel> &tropo_map_levels();
const QVector<TropoMapRegion> &tropo_map_regions();
QUrl tropo_map_url(const TropoMapRegion &region, int forecast_hour, const QDate &date);

}

#endif
