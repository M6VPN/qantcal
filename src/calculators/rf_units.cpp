// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/rf_units.cpp

#include "rf_units.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace qantcal::calculators {

namespace {

constexpr double CENTIMETRES_PER_METRE = 100.0;
constexpr double FEET_PER_METRE = 3.280839895013123;
constexpr double INCHES_PER_FOOT = 12.0;
constexpr double INCHES_PER_METRE = FEET_PER_METRE * INCHES_PER_FOOT;
constexpr double METRES_PER_FOOT = 0.3048;
constexpr double METRES_PER_INCH = 0.0254;
constexpr double MILLIMETRES_PER_METRE = 1000.0;

std::string
fixed_decimal(double value, int precision)
{
	std::ostringstream stream;

	stream << std::fixed << std::setprecision(precision) << value;

	return stream.str();
}

}

double
feet_to_metres(double feet)
{
	return feet * METRES_PER_FOOT;
}

std::string
format_length(double metres, LengthUnit unit)
{
	switch (unit) {
	case LengthUnit::Millimetres:
		return fixed_decimal(metres_to_millimetres(metres), 1) + " mm";
	case LengthUnit::Centimetres:
		return fixed_decimal(metres_to_centimetres(metres), 2) + " cm";
	case LengthUnit::Metres:
		return fixed_decimal(metres, 3) + " m";
	case LengthUnit::FeetInches: {
		const double total_inches = metres * INCHES_PER_METRE;
		const double rounded_inches = std::round(total_inches * 10.0) / 10.0;
		const int feet = static_cast<int>(std::floor(rounded_inches / INCHES_PER_FOOT));
		double inches = rounded_inches - (static_cast<double>(feet) * INCHES_PER_FOOT);

		if (inches >= INCHES_PER_FOOT) {
			inches -= INCHES_PER_FOOT;
			return std::to_string(feet + 1) + " ft " + fixed_decimal(inches, 1) + " in";
		}

		return std::to_string(feet) + " ft " + fixed_decimal(inches, 1) + " in";
	}
	}

	return fixed_decimal(metres, 3) + " m";
}

double
hz_to_mhz(double hz)
{
	return hz / 1000000.0;
}

double
inches_to_metres(double inches)
{
	return inches * METRES_PER_INCH;
}

double
length_unit_to_metres(double value, LengthUnit unit)
{
	switch (unit) {
	case LengthUnit::Millimetres:
		return value / MILLIMETRES_PER_METRE;
	case LengthUnit::Centimetres:
		return value / CENTIMETRES_PER_METRE;
	case LengthUnit::Metres:
		return value;
	case LengthUnit::FeetInches:
		return feet_to_metres(value);
	}

	return value;
}

double
metres_to_centimetres(double metres)
{
	return metres * CENTIMETRES_PER_METRE;
}

double
metres_to_feet(double metres)
{
	return metres * FEET_PER_METRE;
}

double
metres_to_length_unit(double metres, LengthUnit unit)
{
	switch (unit) {
	case LengthUnit::Millimetres:
		return metres_to_millimetres(metres);
	case LengthUnit::Centimetres:
		return metres_to_centimetres(metres);
	case LengthUnit::Metres:
		return metres;
	case LengthUnit::FeetInches:
		return metres_to_feet(metres);
	}

	return metres;
}

double
metres_to_millimetres(double metres)
{
	return metres * MILLIMETRES_PER_METRE;
}

double
mhz_to_hz(double mhz)
{
	return mhz * 1000000.0;
}

LengthUnit
length_unit_from_key(const std::string &key)
{
	if (key == "millimetres")
		return LengthUnit::Millimetres;
	if (key == "centimetres")
		return LengthUnit::Centimetres;
	if (key == "feet_inches")
		return LengthUnit::FeetInches;

	return LengthUnit::Metres;
}

std::string
length_unit_key(LengthUnit unit)
{
	switch (unit) {
	case LengthUnit::Millimetres:
		return "millimetres";
	case LengthUnit::Centimetres:
		return "centimetres";
	case LengthUnit::Metres:
		return "metres";
	case LengthUnit::FeetInches:
		return "feet_inches";
	}

	return "metres";
}

std::string
length_unit_label(LengthUnit unit)
{
	switch (unit) {
	case LengthUnit::Millimetres:
		return "millimetres";
	case LengthUnit::Centimetres:
		return "centimetres";
	case LengthUnit::Metres:
		return "metres";
	case LengthUnit::FeetInches:
		return "feet / inches";
	}

	return "metres";
}

std::string
format_meters(double value)
{
	return format_length(value, LengthUnit::Metres);
}

std::string
format_mhz(double value)
{
	std::ostringstream stream;

	stream << std::fixed << std::setprecision(3) << value << " MHz";

	return stream.str();
}

}
