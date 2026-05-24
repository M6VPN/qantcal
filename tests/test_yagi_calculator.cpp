// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_yagi_calculator.cpp

#include "calculators/yagi_calculator.h"

#include <cassert>
#include <cmath>

namespace {

bool
near(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

qantcal::calculators::YagiDesignResult
calculate(int element_count, qantcal::calculators::YagiPreset preset)
{
	qantcal::calculators::YagiDesignInput input;

	input.frequency_mhz = 144.3;
	input.element_count = element_count;
	input.element_shortening_factor = 0.95;
	input.preset = preset;

	return qantcal::calculators::calculate_yagi(input);
}

void
test_compact_boom_shorter_than_conservative()
{
	const qantcal::calculators::YagiDesignResult compact =
		calculate(5, qantcal::calculators::YagiPreset::Compact);
	const qantcal::calculators::YagiDesignResult conservative =
		calculate(5, qantcal::calculators::YagiPreset::Conservative);

	assert(compact.ok);
	assert(conservative.ok);
	assert(compact.boom_length_metres < conservative.boom_length_metres);
}

void
test_hf_yagi_large_boom_warns()
{
	qantcal::calculators::YagiDesignInput input;

	input.frequency_mhz = 14.2;
	input.element_count = 5;
	const qantcal::calculators::YagiDesignResult result =
		qantcal::calculators::calculate_yagi(input);

	assert(result.ok);
	assert(result.boom_length_metres >= 15.0);
	assert(result.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("impractical")));
}

void
test_invalid_element_count_fails()
{
	assert(!calculate(1, qantcal::calculators::YagiPreset::Conservative).ok);
	assert(!calculate(11, qantcal::calculators::YagiPreset::Conservative).ok);
}

void
test_invalid_frequency_fails()
{
	qantcal::calculators::YagiDesignInput input;

	input.frequency_mhz = 0.0;

	assert(!qantcal::calculators::calculate_yagi(input).ok);
}

void
test_invalid_shortening_factor_fails()
{
	qantcal::calculators::YagiDesignInput input;

	input.frequency_mhz = 144.3;
	input.element_shortening_factor = 0.84;

	assert(!qantcal::calculators::calculate_yagi(input).ok);
}

void
test_boom_correction_reduces_length()
{
	qantcal::calculators::YagiDesignInput input;

	input.frequency_mhz = 144.3;
	input.element_count = 3;

	const qantcal::calculators::YagiDesignResult uncorrected =
		qantcal::calculators::calculate_yagi(input);
	input.boom_correction_metres = 0.01;
	const qantcal::calculators::YagiDesignResult corrected =
		qantcal::calculators::calculate_yagi(input);

	assert(uncorrected.ok);
	assert(corrected.ok);
	assert(near(uncorrected.elements[0].length_metres - corrected.elements[0].length_metres, 0.01, 0.000001));
}

void
test_invalid_boom_correction_fails()
{
	qantcal::calculators::YagiDesignInput input;

	input.frequency_mhz = 144.3;
	input.boom_correction_metres = 10.0;

	assert(!qantcal::calculators::calculate_yagi(input).ok);
}

void
test_long_boom_longer_than_conservative()
{
	const qantcal::calculators::YagiDesignResult long_boom =
		calculate(5, qantcal::calculators::YagiPreset::LongBoom);
	const qantcal::calculators::YagiDesignResult conservative =
		calculate(5, qantcal::calculators::YagiPreset::Conservative);

	assert(long_boom.ok);
	assert(conservative.ok);
	assert(long_boom.boom_length_metres > conservative.boom_length_metres);
}

void
test_missing_element_diameter_warns()
{
	qantcal::calculators::YagiDesignInput input;

	input.frequency_mhz = 144.3;
	input.element_diameter_metres = 0.0;
	const qantcal::calculators::YagiDesignResult result =
		qantcal::calculators::calculate_yagi(input);

	assert(result.ok);
	assert(result.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("Element diameter")));
}

void
test_ten_element_yagi_roles()
{
	const qantcal::calculators::YagiDesignResult result =
		calculate(10, qantcal::calculators::YagiPreset::Conservative);
	int directors = 0;

	assert(result.ok);
	assert(result.elements.size() == 10);
	assert(result.elements[0].role == qantcal::calculators::YagiElementRole::Reflector);
	assert(result.elements[1].role == qantcal::calculators::YagiElementRole::Driven);

	for (const qantcal::calculators::YagiElement &element : result.elements) {
		if (element.role == qantcal::calculators::YagiElementRole::Director)
			++directors;
	}

	assert(directors == 8);
}

void
test_three_element_yagi_sanity()
{
	const qantcal::calculators::YagiDesignResult result =
		calculate(3, qantcal::calculators::YagiPreset::Conservative);

	assert(result.ok);
	assert(result.elements.size() == 3);
	assert(result.elements[0].role == qantcal::calculators::YagiElementRole::Reflector);
	assert(result.elements[1].role == qantcal::calculators::YagiElementRole::Driven);
	assert(result.elements[2].role == qantcal::calculators::YagiElementRole::Director);
	assert(result.elements[0].length_metres > result.elements[1].length_metres);
	assert(result.elements[2].length_metres < result.elements[1].length_metres);
	assert(result.elements[0].position_from_reflector_metres == 0.0);
	assert(result.elements[1].position_from_reflector_metres > result.elements[0].position_from_reflector_metres);
	assert(result.elements[2].position_from_reflector_metres > result.elements[1].position_from_reflector_metres);
	assert(near(result.boom_length_metres, result.elements[2].position_from_reflector_metres, 0.000001));
}

void
test_two_element_yagi_roles()
{
	const qantcal::calculators::YagiDesignResult result =
		calculate(2, qantcal::calculators::YagiPreset::Conservative);

	assert(result.ok);
	assert(result.elements.size() == 2);
	assert(result.elements[0].role == qantcal::calculators::YagiElementRole::Reflector);
	assert(result.elements[1].role == qantcal::calculators::YagiElementRole::Driven);
}

void
test_vhf_compact_yagi_has_no_practicality_warnings()
{
	const qantcal::calculators::YagiDesignResult result =
		calculate(3, qantcal::calculators::YagiPreset::Compact);

	assert(result.ok);
	assert(result.warnings.isEmpty());
}

void
test_unit_formatting_does_not_change_internal_metres()
{
	qantcal::calculators::YagiDesignInput input;

	input.frequency_mhz = 144.3;
	input.preferred_length_unit = qantcal::calculators::LengthUnit::Metres;
	const qantcal::calculators::YagiDesignResult metres =
		qantcal::calculators::calculate_yagi(input);
	input.preferred_length_unit = qantcal::calculators::LengthUnit::Centimetres;
	const qantcal::calculators::YagiDesignResult centimetres =
		qantcal::calculators::calculate_yagi(input);

	assert(metres.ok);
	assert(centimetres.ok);
	assert(near(metres.elements[0].length_metres, centimetres.elements[0].length_metres, 0.000001));
}

}

int
main()
{
	test_compact_boom_shorter_than_conservative();
	test_hf_yagi_large_boom_warns();
	test_invalid_element_count_fails();
	test_invalid_frequency_fails();
	test_invalid_shortening_factor_fails();
	test_boom_correction_reduces_length();
	test_invalid_boom_correction_fails();
	test_long_boom_longer_than_conservative();
	test_missing_element_diameter_warns();
	test_ten_element_yagi_roles();
	test_three_element_yagi_sanity();
	test_two_element_yagi_roles();
	test_vhf_compact_yagi_has_no_practicality_warnings();
	test_unit_formatting_does_not_change_internal_metres();

	return 0;
}
