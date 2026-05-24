// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/settings/app_settings.h

#ifndef QANTCAL_SETTINGS_APP_SETTINGS_H
#define QANTCAL_SETTINGS_APP_SETTINGS_H

#include "calculators/antenna_calculator.h"
#include "calculators/rf_units.h"

#include <QString>

namespace qantcal::settings {

class AppSettings {
public:
	AppSettings();

	calculators::AntennaType antenna_type() const;
	int band_index() const;
	QString language_code() const;
	calculators::LengthUnit length_unit() const;
	double shortening_factor() const;

	void set_antenna_type(calculators::AntennaType antenna_type);
	void set_band_index(int index);
	void set_language_code(const QString &language_code);
	void set_length_unit(calculators::LengthUnit unit);
	void set_shortening_factor(double shortening_factor);
};

}

#endif
