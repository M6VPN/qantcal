// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_lf_mf_reference.cpp

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
test_2200m_lookup()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_name(QStringLiteral("2200m Amateur"), band));
	assert(band.service == qantcal::reference::BandService::Amateur);
	assert(band.category == QStringLiteral("LF"));
	assert(near(band.lower_frequency_mhz, 0.1357));
	assert(near(band.upper_frequency_mhz, 0.1378));
}

void
test_630m_lookup()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_name(QStringLiteral("630m Amateur"), band));
	assert(band.service == qantcal::reference::BandService::Amateur);
	assert(band.category == QStringLiteral("MF"));
	assert(near(band.lower_frequency_mhz, 0.472));
	assert(near(band.upper_frequency_mhz, 0.479));
}

void
test_lw_mw_lookup()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_name(QStringLiteral("LW Broadcast"), band));
	assert(band.service == qantcal::reference::BandService::Broadcast);
	assert(near(band.lower_frequency_mhz, 0.1485));
	assert(near(band.upper_frequency_mhz, 0.2835));
	assert(qantcal::reference::band_reference_by_name(QStringLiteral("MW Broadcast Region 1/3"), band));
	assert(near(band.lower_frequency_mhz, 0.5265));
	assert(near(band.upper_frequency_mhz, 1.6065));
	assert(qantcal::reference::band_reference_by_name(QStringLiteral("MW Broadcast Americas"), band));
	assert(near(band.lower_frequency_mhz, 0.525));
	assert(near(band.upper_frequency_mhz, 1.705));
}

}

int
main()
{
	test_2200m_lookup();
	test_630m_lookup();
	test_lw_mw_lookup();

	return 0;
}
