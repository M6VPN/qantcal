// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/tropo_map_reference.cpp

#include "tropo_map_reference.h"

namespace qantcal::reference {

QString
tropo_cache_file_name(const TropoMapRegion &region, int forecast_hour, const QDate &date)
{
	return QStringLiteral("%1%2_%3.png")
		.arg(region.code)
		.arg(forecast_hour, 3, 10, QLatin1Char('0'))
		.arg(tropo_cache_key(date));
}

QString
tropo_cache_key(const QDate &date)
{
	return date.toString(QStringLiteral("yyyyMMdd"));
}

const QVector<int> &
tropo_forecast_hours()
{
	static const QVector<int> hours = {
		6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42,
		48, 54, 60, 66, 72, 78, 84, 90, 96, 102, 108, 114,
		120, 126, 132, 138, 144
	};

	return hours;
}

const QVector<TropoMapLevel> &
tropo_map_levels()
{
	static const QVector<TropoMapLevel> levels = {
		{ 0, QStringLiteral("#444444"), QStringLiteral("Nil sig") },
		{ 1, QStringLiteral("#8603f1"), QStringLiteral("Marginal") },
		{ 2, QStringLiteral("#01b4ef"), QStringLiteral("Fair") },
		{ 3, QStringLiteral("#02d083"), QStringLiteral("Moderate") },
		{ 4, QStringLiteral("#a5eb01"), QStringLiteral("High") },
		{ 5, QStringLiteral("#efde05"), QStringLiteral("Strong") },
		{ 6, QStringLiteral("#e9b10c"), QStringLiteral("Very strong") },
		{ 7, QStringLiteral("#ff8000"), QStringLiteral("Intense") },
		{ 8, QStringLiteral("#ff0000"), QStringLiteral("Very intense") },
		{ 9, QStringLiteral("#ff80c0"), QStringLiteral("Extreme") },
		{ 10, QStringLiteral("#ffb4dc"), QStringLiteral("Extreme") }
	};

	return levels;
}

const QVector<TropoMapRegion> &
tropo_map_regions()
{
	static const QVector<TropoMapRegion> regions = {
		{ QStringLiteral("enp"), QStringLiteral("Eastern North Pacific"), QStringLiteral("tropo_enp.html") },
		{ QStringLiteral("esp"), QStringLiteral("Eastern South Pacific"), QStringLiteral("tropo_esp.html") },
		{ QStringLiteral("wam"), QStringLiteral("Western North America"), QStringLiteral("tropo_wam.html") },
		{ QStringLiteral("eam"), QStringLiteral("Eastern North America"), QStringLiteral("tropo.html") },
		{ QStringLiteral("car"), QStringLiteral("Gulf-Caribbean"), QStringLiteral("tropo_car.html") },
		{ QStringLiteral("nsa"), QStringLiteral("Northern South America"), QStringLiteral("tropo_nsa.html") },
		{ QStringLiteral("sam"), QStringLiteral("Central South America"), QStringLiteral("tropo_sam.html") },
		{ QStringLiteral("sat"), QStringLiteral("South Atlantic"), QStringLiteral("tropo_sat.html") },
		{ QStringLiteral("nat"), QStringLiteral("North Atlantic"), QStringLiteral("tropo_nat.html") },
		{ QStringLiteral("ent"), QStringLiteral("Eastern North Atlantic"), QStringLiteral("tropo_ent.html") },
		{ QStringLiteral("nwe"), QStringLiteral("Northwestern Europe"), QStringLiteral("tropo_nwe.html") },
		{ QStringLiteral("eur"), QStringLiteral("Europe"), QStringLiteral("tropo_eur.html") },
		{ QStringLiteral("eeu"), QStringLiteral("Eastern Europe"), QStringLiteral("tropo_eeu.html") },
		{ QStringLiteral("afi"), QStringLiteral("South Africa"), QStringLiteral("tropo_afi.html") },
		{ QStringLiteral("mid"), QStringLiteral("Middle East"), QStringLiteral("tropo_mid.html") },
		{ QStringLiteral("nca"), QStringLiteral("North Central Asia"), QStringLiteral("tropo_nca.html") },
		{ QStringLiteral("ino"), QStringLiteral("Indian Ocean"), QStringLiteral("tropo_ino.html") },
		{ QStringLiteral("sea"), QStringLiteral("Southeast Asia"), QStringLiteral("tropo_sea.html") },
		{ QStringLiteral("eas"), QStringLiteral("Far East"), QStringLiteral("tropo_eas.html") },
		{ QStringLiteral("nea"), QStringLiteral("Eastern Siberia"), QStringLiteral("tropo_nea.html") },
		{ QStringLiteral("aus"), QStringLiteral("Australia & New Zealand"), QStringLiteral("tropo_aus.html") },
		{ QStringLiteral("oce"), QStringLiteral("Oceania"), QStringLiteral("tropo_oce.html") },
		{ QStringLiteral("wnp"), QStringLiteral("Western North Pacific"), QStringLiteral("tropo_wnp.html") }
	};

	return regions;
}

QUrl
tropo_map_url(const TropoMapRegion &region, int forecast_hour, const QDate &date)
{
	return QUrl(QStringLiteral("https://www.dxinfocentre.com/tr_map/fcst/%1%2.png?v%3")
		.arg(region.code)
		.arg(forecast_hour, 3, 10, QLatin1Char('0'))
		.arg(tropo_cache_key(date)));
}

}
