// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_coax_loss_calculator.cpp

#include "calculators/coax_loss_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_high_loss_warns()
{
	qantcal::calculators::CoaxLossInput input;

	input.frequency_mhz = 144.3;
	input.length_metres = 100.0;
	input.loss_db_per_100m = 6.5;
	input.input_power_watts = 100.0;
	const qantcal::calculators::CoaxLossResult result =
		qantcal::calculators::calculate_coax_loss(input);

	assert(result.ok);
	assert(!result.warnings.isEmpty());
}

void
test_invalid_inputs_fail()
{
	qantcal::calculators::CoaxLossInput input;

	input.frequency_mhz = 0.0;
	assert(!qantcal::calculators::calculate_coax_loss(input).ok);

	input.frequency_mhz = 144.3;
	input.length_metres = -1.0;
	assert(!qantcal::calculators::calculate_coax_loss(input).ok);

	input.length_metres = 10.0;
	input.loss_db_per_100m = -0.1;
	assert(!qantcal::calculators::calculate_coax_loss(input).ok);

	input.loss_db_per_100m = 1.0;
	input.input_power_watts = -1.0;
	assert(!qantcal::calculators::calculate_coax_loss(input).ok);

	input.input_power_watts = 100.0;
	input.swr = 0.9;
	assert(!qantcal::calculators::calculate_coax_loss(input).ok);
}

void
test_matched_loss()
{
	qantcal::calculators::CoaxLossInput input;

	input.frequency_mhz = 144.3;
	input.length_metres = 50.0;
	input.loss_db_per_100m = 3.0;
	input.input_power_watts = 100.0;
	const qantcal::calculators::CoaxLossResult result =
		qantcal::calculators::calculate_coax_loss(input);

	assert(result.ok);
	assert(near(result.matched_loss_db, 1.5, 0.000001));
	assert(near(result.total_loss_db, 1.5, 0.000001));
	assert(near(result.delivered_power_watts, 70.794, 0.010));
}

void
test_swr_adds_loss()
{
	qantcal::calculators::CoaxLossInput input;

	input.frequency_mhz = 144.3;
	input.length_metres = 50.0;
	input.loss_db_per_100m = 3.0;
	input.input_power_watts = 100.0;
	input.swr = 3.0;
	const qantcal::calculators::CoaxLossResult result =
		qantcal::calculators::calculate_coax_loss(input);

	assert(result.ok);
	assert(result.total_loss_db > result.matched_loss_db);
	assert(result.additional_swr_loss_db > 0.0);
	assert(result.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("High SWR")));
}

void
test_zero_length_is_lossless()
{
	qantcal::calculators::CoaxLossInput input;

	input.frequency_mhz = 14.2;
	input.length_metres = 0.0;
	input.loss_db_per_100m = 3.0;
	input.input_power_watts = 100.0;
	const qantcal::calculators::CoaxLossResult result =
		qantcal::calculators::calculate_coax_loss(input);

	assert(result.ok);
	assert(near(result.total_loss_db, 0.0, 0.000001));
	assert(near(result.delivered_power_watts, 100.0, 0.000001));
}

}

int
main()
{
	test_high_loss_warns();
	test_invalid_inputs_fail();
	test_matched_loss();
	test_swr_adds_loss();
	test_zero_length_is_lossless();

	return 0;
}
