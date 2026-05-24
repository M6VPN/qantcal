// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/lf_mf_antenna_calculator.h

#ifndef QANTCAL_CALCULATORS_LF_MF_ANTENNA_CALCULATOR_H
#define QANTCAL_CALCULATORS_LF_MF_ANTENNA_CALCULATOR_H

#include "loading_coil_calculator.h"
#include "rf_units.h"

#include <QString>
#include <QStringList>

namespace qantcal::calculators {

enum class LfMfDesignType {
	FullSizeReference,
	ShortLoadedVertical,
	InvertedL,
	TopLoadedT,
	ReceiveOnlyCompact
};

struct LfMfAntennaInput {
	LfMfDesignType design_type = LfMfDesignType::FullSizeReference;
	LengthUnit preferred_length_unit = LengthUnit::Metres;
	double frequency_mhz = 0.0;
	double vertical_height_metres = 0.0;
	double horizontal_or_top_length_metres = 0.0;
	double estimated_capacitance_pf = 0.0;
	bool has_estimated_capacitance = false;
};

struct LfMfAntennaResult {
	bool ok = false;
	bool receive_only = false;
	bool loading_likely_required = false;
	double frequency_mhz = 0.0;
	double frequency_khz = 0.0;
	double wavelength_metres = 0.0;
	double quarter_wave_metres = 0.0;
	double half_wave_metres = 0.0;
	double full_wave_metres = 0.0;
	double electrical_height_ratio = 0.0;
	double total_wire_length_metres = 0.0;
	double total_wire_ratio = 0.0;
	LoadingCoilResult loading_coil;
	QString error_message;
	QStringList dimensions;
	QStringList notes;
	QStringList warnings;
};

LfMfAntennaResult calculate_lf_mf_antenna(const LfMfAntennaInput &input);
QString lf_mf_design_type_key(LfMfDesignType design_type);
QString lf_mf_design_type_label(LfMfDesignType design_type);
LfMfDesignType lf_mf_design_type_from_key(const QString &key);

}

#endif
