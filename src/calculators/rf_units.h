// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/rf_units.h

#ifndef QANTCAL_CALCULATORS_RF_UNITS_H
#define QANTCAL_CALCULATORS_RF_UNITS_H

#include <string>

namespace qantcal::calculators {

enum class LengthUnit {
	Millimetres,
	Centimetres,
	Metres,
	FeetInches
};

double feet_to_metres(double feet);
std::string format_length(double metres, LengthUnit unit);
double hz_to_mhz(double hz);
double inches_to_metres(double inches);
double length_unit_to_metres(double value, LengthUnit unit);
double metres_to_centimetres(double metres);
double metres_to_feet(double metres);
double metres_to_length_unit(double metres, LengthUnit unit);
double metres_to_millimetres(double metres);
double mhz_to_hz(double mhz);
LengthUnit length_unit_from_key(const std::string &key);
std::string length_unit_key(LengthUnit unit);
std::string length_unit_label(LengthUnit unit);
bool parse_feet_inches(const std::string &text, double &feet);
std::string format_meters(double value);
std::string format_mhz(double value);

}

#endif
