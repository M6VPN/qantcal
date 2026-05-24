// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/impedance_calculator.h

#ifndef QANTCAL_CALCULATORS_IMPEDANCE_CALCULATOR_H
#define QANTCAL_CALCULATORS_IMPEDANCE_CALCULATOR_H

#include <QString>
#include <QStringList>

namespace qantcal::calculators {

struct ImpedanceCalculationInput {
	double frequency_mhz = 0.0;
	double reactance_ohms = 0.0;
	double resistance_ohms = 0.0;
	double system_impedance_ohms = 50.0;
};

struct ImpedanceCalculationResult {
	bool has_reactive_component = false;
	bool is_perfect_match = false;
	bool is_total_reflection = false;
	bool ok = false;
	bool reactance_is_inductive = false;
	double admittance_magnitude_siemens = 0.0;
	double equivalent_capacitance_pf = 0.0;
	double equivalent_inductance_uh = 0.0;
	double conductance_siemens = 0.0;
	double impedance_magnitude_ohms = 0.0;
	double mismatch_loss_db = 0.0;
	double phase_degrees = 0.0;
	double reflection_coefficient_magnitude = 0.0;
	double return_loss_db = 0.0;
	double swr = 1.0;
	double susceptance_siemens = 0.0;
	QString error_message;
	QString note;
	QStringList warnings;
};

ImpedanceCalculationResult calculate_impedance(const ImpedanceCalculationInput &input);

}

#endif
