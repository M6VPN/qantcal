// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/choke_calculator.h

#ifndef QANTCAL_CALCULATORS_CHOKE_CALCULATOR_H
#define QANTCAL_CALCULATORS_CHOKE_CALCULATOR_H

#include <QString>
#include <QStringList>

namespace qantcal::calculators {

enum class ChokeCoreMix {
	Custom,
	Mix31,
	Mix43,
	Mix61,
	AirCore
};

struct ChokeCalculationInput {
	ChokeCoreMix mix = ChokeCoreMix::Custom;
	double frequency_mhz = 0.0;
	double reference_impedance_ohms = 0.0;
	double target_impedance_ohms = 0.0;
	double turns = 0.0;
};

struct ChokeCalculationResult {
	bool ok = false;
	int suggested_turns = 0;
	double estimated_impedance_ohms = 0.0;
	double target_ratio = 0.0;
	QString error_message;
	QString mix_note;
	QString note;
	QStringList warnings;
};

ChokeCalculationResult calculate_choke(const ChokeCalculationInput &input);
QString choke_mix_label(ChokeCoreMix mix);

}

#endif
