// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/rf_units.cpp

#include "rf_units.h"

#include <iomanip>
#include <sstream>

namespace qantcal::calculators {

double
hz_to_mhz(double hz)
{
	return hz / 1000000.0;
}

double
mhz_to_hz(double mhz)
{
	return mhz * 1000000.0;
}

std::string
format_meters(double value)
{
	std::ostringstream stream;

	stream << std::fixed << std::setprecision(3) << value << " m";

	return stream.str();
}

std::string
format_mhz(double value)
{
	std::ostringstream stream;

	stream << std::fixed << std::setprecision(3) << value << " MHz";

	return stream.str();
}

}
