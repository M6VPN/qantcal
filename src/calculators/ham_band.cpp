// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/ham_band.cpp

#include "ham_band.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace qantcal::calculators {

namespace {

std::string
normalised_name(const std::string &name)
{
	std::string output;

	for (const char character : name) {
		if (!std::isspace(static_cast<unsigned char>(character)))
			output.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
	}

	return output;
}

}

const HamBand *
find_ham_band_by_frequency(double frequency_mhz)
{
	if (!std::isfinite(frequency_mhz))
		return nullptr;

	for (const HamBand &band : ham_bands()) {
		if (frequency_mhz >= band.lower_frequency_mhz && frequency_mhz <= band.upper_frequency_mhz)
			return &band;
	}

	return nullptr;
}

const HamBand *
find_ham_band_by_name(const std::string &name)
{
	const std::string wanted_name = normalised_name(name);

	for (const HamBand &band : ham_bands()) {
		if (normalised_name(band.display_name) == wanted_name)
			return &band;
	}

	return nullptr;
}

const std::vector<HamBand> &
ham_bands()
{
	static const std::vector<HamBand> bands = {
		{ "160m", 1.8, 2.0, 1.9, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "80m", 3.5, 4.0, 3.65, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "60m", 5.0, 5.5, 5.35, "Convenience reference only. 60m allocations vary heavily by country and licence." },
		{ "40m", 7.0, 7.3, 7.1, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "30m", 10.1, 10.15, 10.125, "Convenience reference only. Check mode, power, and national restrictions." },
		{ "20m", 14.0, 14.35, 14.2, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "17m", 18.068, 18.168, 18.1, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "15m", 21.0, 21.45, 21.2, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "12m", 24.89, 24.99, 24.95, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "10m", 28.0, 29.7, 28.5, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "6m", 50.0, 54.0, 50.15, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "4m", 70.0, 70.5, 70.2, "Convenience reference only. 4m availability varies by country." },
		{ "2m", 144.0, 148.0, 145.0, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
		{ "70cm", 430.0, 440.0, 433.0, "Convenience reference only. Check your national licence, current band plan, and local restrictions." },
	};

	return bands;
}

}
