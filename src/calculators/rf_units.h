// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/rf_units.h

#ifndef QANTCAL_CALCULATORS_RF_UNITS_H
#define QANTCAL_CALCULATORS_RF_UNITS_H

#include <string>

namespace qantcal::calculators {

double hz_to_mhz(double hz);
double mhz_to_hz(double mhz);
std::string format_meters(double value);
std::string format_mhz(double value);

}

#endif
