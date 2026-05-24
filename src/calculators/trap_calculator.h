// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/trap_calculator.h

#ifndef QANTCAL_CALCULATORS_TRAP_CALCULATOR_H
#define QANTCAL_CALCULATORS_TRAP_CALCULATOR_H

#include <QString>
#include <QStringList>

namespace qantcal::calculators {

enum class TrapCalculationMode {
	FrequencyFromLC,
	InductanceFromFrequencyCapacitance,
	CapacitanceFromFrequencyInductance
};

struct TrapCalculationInput {
	TrapCalculationMode mode = TrapCalculationMode::FrequencyFromLC;
	double capacitance_pf = 0.0;
	double frequency_mhz = 0.0;
	double inductance_uh = 0.0;
	double operating_frequency_mhz = 0.0;
};

struct TrapCalculationResult {
	bool ok = false;
	bool has_operating_reactance = false;
	double capacitance_pf = 0.0;
	double frequency_hz = 0.0;
	double frequency_khz = 0.0;
	double frequency_mhz = 0.0;
	double inductance_uh = 0.0;
	double operating_capacitive_reactance_ohms = 0.0;
	double operating_frequency_mhz = 0.0;
	double operating_inductive_reactance_ohms = 0.0;
	double reactance_ohms = 0.0;
	QString error_message;
	QString note;
	QStringList warnings;
};

TrapCalculationResult calculate_trap(const TrapCalculationInput &input);

}

#endif
