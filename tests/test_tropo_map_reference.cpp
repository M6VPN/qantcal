// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_tropo_map_reference.cpp

#include "reference/tropo_map_reference.h"

#include <cassert>

namespace {

void
test_colour_key_matches_hepburn_scale()
{
	const QVector<qantcal::reference::TropoMapLevel> &levels = qantcal::reference::tropo_map_levels();

	assert(levels.size() == 11);
	assert(levels.first().index == 0);
	assert(levels.first().label == QStringLiteral("Nil sig"));
	assert(levels.last().index == 10);
	assert(levels.last().colour_hex == QStringLiteral("#ffb4dc"));
}

void
test_forecast_hours_match_page_sequence()
{
	const QVector<int> &hours = qantcal::reference::tropo_forecast_hours();

	assert(hours.size() == 30);
	assert(hours.first() == 6);
	assert(hours[12] == 42);
	assert(hours[13] == 48);
	assert(hours.last() == 144);
}

void
test_regions_include_examples()
{
	const QVector<qantcal::reference::TropoMapRegion> &regions = qantcal::reference::tropo_map_regions();
	bool found_europe = false;
	bool found_north_pacific = false;

	for (const qantcal::reference::TropoMapRegion &region : regions) {
		found_europe = found_europe || (region.code == QStringLiteral("eur") && region.page_path == QStringLiteral("tropo_eur.html"));
		found_north_pacific = found_north_pacific || (region.code == QStringLiteral("enp") && region.page_path == QStringLiteral("tropo_enp.html"));
	}

	assert(regions.size() == 23);
	assert(found_europe);
	assert(found_north_pacific);
}

void
test_url_and_cache_schema()
{
	const qantcal::reference::TropoMapRegion region{
		QStringLiteral("eur"),
		QStringLiteral("Europe"),
		QStringLiteral("tropo_eur.html")
	};
	const QDate date(2026, 5, 24);

	assert(qantcal::reference::tropo_cache_key(date) == QStringLiteral("20260524"));
	assert(qantcal::reference::tropo_cache_file_name(region, 6, date) == QStringLiteral("eur006_20260524.png"));
	assert(qantcal::reference::tropo_map_url(region, 6, date).toString() == QStringLiteral("https://www.dxinfocentre.com/tr_map/fcst/eur006.png?v20260524"));
}

}

int
main()
{
	test_colour_key_matches_hepburn_scale();
	test_forecast_hours_match_page_sequence();
	test_regions_include_examples();
	test_url_and_cache_schema();

	return 0;
}
