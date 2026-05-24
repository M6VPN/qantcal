// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/coax_loss_calculator.h

#ifndef QANTCAL_CALCULATORS_COAX_LOSS_CALCULATOR_H
#define QANTCAL_CALCULATORS_COAX_LOSS_CALCULATOR_H

#include <QString>
#include <QStringList>

namespace qantcal::calculators {

struct CoaxLossInput {
	double frequency_mhz = 0.0;
	double length_metres = 0.0;
	double loss_db_per_100m = 0.0;
	double input_power_watts = 0.0;
	double swr = 1.0;
};

struct CoaxLossResult {
	bool ok = false;
	double additional_swr_loss_db = 0.0;
	double delivered_power_watts = 0.0;
	double loss_percent = 0.0;
	double matched_loss_db = 0.0;
	double total_loss_db = 0.0;
	QString error_message;
	QString note;
	QStringList warnings;
};

CoaxLossResult calculate_coax_loss(const CoaxLossInput &input);

}

#endif
