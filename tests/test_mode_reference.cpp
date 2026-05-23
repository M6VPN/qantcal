// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_mode_reference.cpp

#include "reference/mode_reference.h"

#include <cassert>

namespace {

void
test_each_mode_has_notes()
{
	for (const qantcal::reference::ModeReference &mode : qantcal::reference::mode_references()) {
		assert(!mode.name.isEmpty());
		assert(!mode.bandwidth_category.isEmpty());
		assert(!mode.use_notes.isEmpty());
		assert(!mode.weak_signal_notes.isEmpty());
		assert(!mode.caution.isEmpty());
	}
}

void
test_lookup_mode()
{
	qantcal::reference::ModeReference mode;

	assert(qantcal::reference::mode_reference_by_key(QStringLiteral("cw"), mode));
	assert(mode.mode == qantcal::reference::ModeType::Cw);
}

}

int
main()
{
	test_each_mode_has_notes();
	test_lookup_mode();

	return 0;
}
