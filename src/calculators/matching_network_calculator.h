// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/matching_network_calculator.h

#ifndef QANTCAL_CALCULATORS_MATCHING_NETWORK_CALCULATOR_H
#define QANTCAL_CALCULATORS_MATCHING_NETWORK_CALCULATOR_H

#include <QString>
#include <QStringList>

namespace qantcal::calculators {

struct MatchingNetworkInput {
	double frequency_mhz = 0.0;
	double load_resistance_ohms = 0.0;
	double source_resistance_ohms = 0.0;
};

struct MatchingNetworkResult {
	bool ok = false;
	bool no_network_needed = false;
	bool source_is_low_resistance_side = false;
	double high_resistance_ohms = 0.0;
	double high_pass_series_capacitance_pf = 0.0;
	double high_pass_shunt_inductance_uh = 0.0;
	double load_resistance_ohms = 0.0;
	double low_pass_series_inductance_uh = 0.0;
	double low_pass_shunt_capacitance_pf = 0.0;
	double low_resistance_ohms = 0.0;
	double q = 0.0;
	double ratio = 0.0;
	double series_reactance_ohms = 0.0;
	double shunt_reactance_ohms = 0.0;
	double source_resistance_ohms = 0.0;
	QString error_message;
	QString high_side_label;
	QString low_side_label;
	QString note;
	QStringList warnings;
};

MatchingNetworkResult calculate_matching_network(const MatchingNetworkInput &input);

}

#endif
