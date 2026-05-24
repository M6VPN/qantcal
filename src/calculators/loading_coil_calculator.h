// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/loading_coil_calculator.h

#ifndef QANTCAL_CALCULATORS_LOADING_COIL_CALCULATOR_H
#define QANTCAL_CALCULATORS_LOADING_COIL_CALCULATOR_H

#include <QString>
#include <QStringList>

namespace qantcal::calculators {

struct LoadingCoilInput {
	double frequency_mhz = 0.0;
	double physical_vertical_height_metres = 0.0;
	double top_loading_length_metres = 0.0;
	double estimated_capacitance_pf = 0.0;
	bool has_estimated_capacitance = false;
};

struct LoadingCoilResult {
	bool ok = false;
	bool loading_likely_required = false;
	bool has_inductance = false;
	double wavelength_metres = 0.0;
	double electrical_height_ratio = 0.0;
	double inductance_h = 0.0;
	double inductance_uh = 0.0;
	double inductance_mh = 0.0;
	QString error_message;
	QStringList warnings;
};

LoadingCoilResult calculate_loading_coil(const LoadingCoilInput &input);

}

#endif
