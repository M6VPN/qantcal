// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_lf_mf_antenna_calculator.cpp

#include "calculators/lf_mf_antenna_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_full_size_reference_lengths()
{
	qantcal::calculators::LfMfAntennaInput input;

	input.frequency_mhz = 0.475;
	const qantcal::calculators::LfMfAntennaResult result =
		qantcal::calculators::calculate_lf_mf_antenna(input);

	assert(result.ok);
	assert(near(result.half_wave_metres, 315.57, 0.5));
}

void
test_quarter_wave_2200m_reference()
{
	qantcal::calculators::LfMfAntennaInput input;

	input.frequency_mhz = 0.1365;
	const qantcal::calculators::LfMfAntennaResult result =
		qantcal::calculators::calculate_lf_mf_antenna(input);

	assert(result.ok);
	assert(near(result.quarter_wave_metres, 549.07, 1.0));
}

void
test_short_vertical_warns()
{
	qantcal::calculators::LfMfAntennaInput input;

	input.design_type = qantcal::calculators::LfMfDesignType::ShortLoadedVertical;
	input.frequency_mhz = 0.475;
	input.vertical_height_metres = 10.0;
	const qantcal::calculators::LfMfAntennaResult result =
		qantcal::calculators::calculate_lf_mf_antenna(input);

	assert(result.ok);
	assert(result.loading_likely_required);
	assert(result.electrical_height_ratio < 0.05);
	assert(result.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("electrically short")));
}

void
test_invalid_inputs()
{
	qantcal::calculators::LfMfAntennaInput input;

	input.frequency_mhz = -0.475;
	assert(!qantcal::calculators::calculate_lf_mf_antenna(input).ok);
	input.frequency_mhz = 0.475;
	input.vertical_height_metres = -1.0;
	assert(!qantcal::calculators::calculate_lf_mf_antenna(input).ok);
}

}

int
main()
{
	test_full_size_reference_lengths();
	test_invalid_inputs();
	test_quarter_wave_2200m_reference();
	test_short_vertical_warns();

	return 0;
}
