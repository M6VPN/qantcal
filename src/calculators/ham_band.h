// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/ham_band.h

#ifndef QANTCAL_CALCULATORS_HAM_BAND_H
#define QANTCAL_CALCULATORS_HAM_BAND_H

#include <string>
#include <vector>

namespace qantcal::calculators {

struct HamBand {
	std::string display_name;
	double lower_frequency_mhz = 0.0;
	double upper_frequency_mhz = 0.0;
	double design_frequency_mhz = 0.0;
	std::string notes;
};

const HamBand *find_ham_band_by_frequency(double frequency_mhz);
const HamBand *find_ham_band_by_name(const std::string &name);
const std::vector<HamBand> &ham_bands();

}

#endif
