// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_ham_band.cpp

#include "calculators/ham_band.h"

#include <cassert>

void
test_band_lookup_by_frequency()
{
	const qantcal::calculators::HamBand *band =
		qantcal::calculators::find_ham_band_by_frequency(7.1);

	assert(band != nullptr);
	assert(band->display_name == "40m");
	assert(band->design_frequency_mhz == 7.1);
}

void
test_band_lookup_by_name()
{
	const qantcal::calculators::HamBand *band =
		qantcal::calculators::find_ham_band_by_name("70 cm");

	assert(band != nullptr);
	assert(band->display_name == "70cm");
	assert(band->lower_frequency_mhz == 430.0);
}

int
main()
{
	test_band_lookup_by_frequency();
	test_band_lookup_by_name();

	return 0;
}
