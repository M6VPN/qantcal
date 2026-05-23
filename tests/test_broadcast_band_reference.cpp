// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_broadcast_band_reference.cpp

#include "reference/band_reference.h"

#include <cassert>
#include <cmath>

namespace {

bool
near(double actual, double expected)
{
	return std::fabs(actual - expected) <= 0.000001;
}

void
test_48m_informal_lookup()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_name(QStringLiteral("48m Informal"), band));
	assert(band.service == qantcal::reference::BandService::Informal);
	assert(near(band.lower_frequency_mhz, 6.200));
	assert(near(band.upper_frequency_mhz, 6.500));
	assert(near(band.design_frequency_mhz, 6.300));
	assert(band.use_case_notes.contains(QStringLiteral("listening")));
}

void
test_49m_broadcast_lookup()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_name(QStringLiteral("49m Broadcast"), band));
	assert(band.service == qantcal::reference::BandService::Broadcast);
	assert(near(band.lower_frequency_mhz, 5.900));
	assert(near(band.upper_frequency_mhz, 6.200));
	assert(near(band.design_frequency_mhz, 6.050));
}

void
test_75m_broadcast_lookup()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_name(QStringLiteral("75m Broadcast"), band));
	assert(band.service == qantcal::reference::BandService::Broadcast);
	assert(near(band.lower_frequency_mhz, 3.900));
	assert(near(band.upper_frequency_mhz, 4.000));
	assert(near(band.design_frequency_mhz, 3.950));
}

void
test_frequency_lookup()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_frequency(6.050, band));
	assert(band.name == QStringLiteral("49m Broadcast"));
	assert(qantcal::reference::band_reference_by_frequency(6.300, band));
	assert(band.name == QStringLiteral("48m Informal"));
}

}

int
main()
{
	test_48m_informal_lookup();
	test_49m_broadcast_lookup();
	test_75m_broadcast_lookup();
	test_frequency_lookup();

	return 0;
}
