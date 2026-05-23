// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_propagation_notes.cpp

#include "reference/propagation_notes.h"

#include <cassert>

namespace {

void
test_frequency_lookup()
{
	qantcal::reference::PropagationProfile profile;

	assert(qantcal::reference::propagation_profile_by_frequency(7.1, profile));
	assert(profile.band_name == QStringLiteral("40m"));
	assert(!profile.categories.isEmpty());
	assert(!profile.variability.isEmpty());
}

void
test_name_lookup()
{
	qantcal::reference::PropagationProfile profile;

	assert(qantcal::reference::propagation_profile_by_band_name(QStringLiteral("6m"), profile));
	assert(profile.categories.contains(QStringLiteral("sporadic-E")));
}

}

int
main()
{
	test_frequency_lookup();
	test_name_lookup();

	return 0;
}
