// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_radio_horizon_calculator.cpp

#include "calculators/radio_horizon_calculator.h"
#include "calculators/rf_units.h"

#include <cassert>
#include <cmath>

namespace {

bool
near_value(double actual, double expected, double tolerance)
{
	return std::fabs(actual - expected) <= tolerance;
}

void
test_combined_horizon()
{
	qantcal::calculators::RadioHorizonInput input;

	input.tx_height_m = 10.0;
	input.rx_height_m = 10.0;

	const qantcal::calculators::RadioHorizonResult result =
		qantcal::calculators::calculate_radio_horizon(input);

	assert(result.ok);
	assert(near_value(result.combined_distance_km, 26.077, 0.010));
	assert(result.model_label.find("4/3") != std::string::npos);
}

void
test_height_unit_conversion()
{
	qantcal::calculators::RadioHorizonInput metres_input;
	qantcal::calculators::RadioHorizonInput centimetres_input;

	metres_input.tx_height_m = 10.0;
	metres_input.rx_height_m = 10.0;
	centimetres_input.tx_height_m = qantcal::calculators::length_unit_to_metres(1000.0, qantcal::calculators::LengthUnit::Centimetres);
	centimetres_input.rx_height_m = qantcal::calculators::length_unit_to_metres(1000.0, qantcal::calculators::LengthUnit::Centimetres);

	const qantcal::calculators::RadioHorizonResult metres_result =
		qantcal::calculators::calculate_radio_horizon(metres_input);
	const qantcal::calculators::RadioHorizonResult centimetres_result =
		qantcal::calculators::calculate_radio_horizon(centimetres_input);

	assert(metres_result.ok);
	assert(centimetres_result.ok);
	assert(near_value(metres_result.combined_distance_km, centimetres_result.combined_distance_km, 0.001));
}

void
test_ten_metre_horizon()
{
	qantcal::calculators::RadioHorizonInput input;

	input.tx_height_m = 10.0;
	input.rx_height_m = 0.0;

	const qantcal::calculators::RadioHorizonResult result =
		qantcal::calculators::calculate_radio_horizon(input);

	assert(result.ok);
	assert(near_value(result.tx_horizon_km, std::sqrt(170.0), 0.001));
}

void
test_geometric_horizon_model()
{
	qantcal::calculators::RadioHorizonInput input;

	input.tx_height_m = 10.0;
	input.rx_height_m = 0.0;
	input.model = qantcal::calculators::RadioHorizonModel::Geometric;

	const qantcal::calculators::RadioHorizonResult result =
		qantcal::calculators::calculate_radio_horizon(input);

	assert(result.ok);
	assert(near_value(result.tx_horizon_km, std::sqrt(127.46), 0.001));
	assert(result.model_label.find("Geometric") != std::string::npos);
}

}

int
main()
{
	test_combined_horizon();
	test_geometric_horizon_model();
	test_height_unit_conversion();
	test_ten_metre_horizon();

	return 0;
}
