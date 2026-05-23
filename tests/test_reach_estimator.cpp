// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_reach_estimator.cpp

#include "reference/reach_estimator.h"

#include "calculators/rf_units.h"

#include <cassert>
#include <cmath>

namespace {

bool
near(double actual, double expected)
{
	return std::fabs(actual - expected) < 0.001;
}

void
test_hf_has_no_fixed_km_prediction()
{
	qantcal::reference::ReachEstimateInput input;

	input.frequency_mhz = 14.2;
	input.tx_height_metres = 10.0;
	input.rx_height_metres = 10.0;
	const qantcal::reference::ReachEstimateResult result = qantcal::reference::estimate_reach(input);

	assert(result.ok);
	assert(!result.includes_radio_horizon);
	assert(result.summary.contains(QStringLiteral("does not calculate fixed kilometre predictions")));
}

void
test_invalid_negative_height_fails()
{
	qantcal::reference::ReachEstimateInput input;

	input.frequency_mhz = 144.3;
	input.tx_height_metres = -1.0;
	input.rx_height_metres = 10.0;

	assert(!qantcal::reference::estimate_reach(input).ok);
}

void
test_invalid_negative_power_fails()
{
	qantcal::reference::ReachEstimateInput input;

	input.frequency_mhz = 144.3;
	input.tx_height_metres = 10.0;
	input.rx_height_metres = 10.0;
	input.has_power_watts = true;
	input.power_watts = -5.0;

	assert(!qantcal::reference::estimate_reach(input).ok);
}

void
test_unit_conversion_does_not_change_internal_result()
{
	qantcal::reference::ReachEstimateInput metres_input;
	qantcal::reference::ReachEstimateInput feet_input;

	metres_input.frequency_mhz = 144.3;
	metres_input.tx_height_metres = 10.0;
	metres_input.rx_height_metres = 10.0;
	feet_input = metres_input;
	feet_input.tx_height_metres = qantcal::calculators::feet_to_metres(qantcal::calculators::metres_to_feet(10.0));
	feet_input.rx_height_metres = qantcal::calculators::feet_to_metres(qantcal::calculators::metres_to_feet(10.0));

	const qantcal::reference::ReachEstimateResult metres = qantcal::reference::estimate_reach(metres_input);
	const qantcal::reference::ReachEstimateResult feet = qantcal::reference::estimate_reach(feet_input);

	assert(metres.ok);
	assert(feet.ok);
	assert(near(metres.combined_horizon_km, feet.combined_horizon_km));
}

void
test_vhf_includes_radio_horizon()
{
	qantcal::reference::ReachEstimateInput input;

	input.frequency_mhz = 144.3;
	input.tx_height_metres = 10.0;
	input.rx_height_metres = 10.0;
	const qantcal::reference::ReachEstimateResult result = qantcal::reference::estimate_reach(input);

	assert(result.ok);
	assert(result.includes_radio_horizon);
	assert(result.combined_horizon_km > 0.0);
}

}

int
main()
{
	test_hf_has_no_fixed_km_prediction();
	test_invalid_negative_height_fails();
	test_invalid_negative_power_fails();
	test_unit_conversion_does_not_change_internal_result();
	test_vhf_includes_radio_horizon();

	return 0;
}
