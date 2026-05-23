// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_band_reference.cpp

#include "reference/band_reference.h"

#include <cassert>

namespace {

void
test_all_bands_have_notes()
{
	for (const qantcal::reference::BandReference &band : qantcal::reference::band_references()) {
		assert(!band.name.isEmpty());
		assert(!band.antenna_notes.isEmpty());
		assert(!band.propagation_notes.isEmpty());
		assert(!band.warning.isEmpty());
	}
}

void
test_lookup_by_frequency()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_frequency(14.2, band));
	assert(band.name == QStringLiteral("20m"));
}

void
test_lookup_by_name()
{
	qantcal::reference::BandReference band;

	assert(qantcal::reference::band_reference_by_name(QStringLiteral("2m"), band));
	assert(band.design_frequency_mhz == 144.3);
}

void
test_unknown_frequency_fails_cleanly()
{
	qantcal::reference::BandReference band;

	assert(!qantcal::reference::band_reference_by_frequency(9999.0, band));
}

}

int
main()
{
	test_all_bands_have_notes();
	test_lookup_by_frequency();
	test_lookup_by_name();
	test_unknown_frequency_fails_cleanly();

	return 0;
}
