// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/settings/app_settings.cpp

#include "app_settings.h"

#include <QSettings>

namespace qantcal::settings {

namespace {

constexpr const char *ANTENNA_TYPE_KEY = "calculator/antenna_type";
constexpr const char *BAND_INDEX_KEY = "calculator/band_index";
constexpr const char *LENGTH_UNIT_KEY = "calculator/length_unit";
constexpr const char *SHORTENING_FACTOR_KEY = "calculator/shortening_factor";

}

AppSettings::AppSettings()
{
}

calculators::AntennaType
AppSettings::antenna_type() const
{
	QSettings settings;

	return static_cast<calculators::AntennaType>(
		settings.value(ANTENNA_TYPE_KEY, static_cast<int>(calculators::AntennaType::HalfWaveDipole)).toInt()
	);
}

int
AppSettings::band_index() const
{
	QSettings settings;

	return settings.value(BAND_INDEX_KEY, 4).toInt();
}

calculators::LengthUnit
AppSettings::length_unit() const
{
	QSettings settings;
	const std::string key = settings.value(LENGTH_UNIT_KEY, QStringLiteral("metres")).toString().toStdString();

	return calculators::length_unit_from_key(key);
}

double
AppSettings::shortening_factor() const
{
	QSettings settings;

	return settings.value(SHORTENING_FACTOR_KEY, calculators::DEFAULT_WIRE_FACTOR).toDouble();
}

void
AppSettings::set_antenna_type(calculators::AntennaType antenna_type)
{
	QSettings settings;

	settings.setValue(ANTENNA_TYPE_KEY, static_cast<int>(antenna_type));
}

void
AppSettings::set_band_index(int index)
{
	QSettings settings;

	settings.setValue(BAND_INDEX_KEY, index);
}

void
AppSettings::set_length_unit(calculators::LengthUnit unit)
{
	QSettings settings;

	settings.setValue(LENGTH_UNIT_KEY, QString::fromStdString(calculators::length_unit_key(unit)));
}

void
AppSettings::set_shortening_factor(double shortening_factor)
{
	QSettings settings;

	settings.setValue(SHORTENING_FACTOR_KEY, shortening_factor);
}

}
