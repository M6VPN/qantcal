// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/propagation_notes.cpp

#include "propagation_notes.h"

#include "band_reference.h"

namespace qantcal::reference {

namespace {

PropagationProfile
make_profile(
	const QString &band_name,
	const QStringList &categories,
	const QString &day_night_tendency,
	const QString &character,
	const QString &noise_tendency,
	const QString &antenna_practicality,
	const QString &variability
)
{
	PropagationProfile profile;

	profile.band_name = band_name;
	profile.categories = categories;
	profile.day_night_tendency = day_night_tendency;
	profile.character = character;
	profile.noise_tendency = noise_tendency;
	profile.antenna_practicality = antenna_practicality;
	profile.variability = variability;

	return profile;
}

const QVector<PropagationProfile> &
profiles()
{
	static const QVector<PropagationProfile> data = {
		make_profile(QStringLiteral("160m"), { QStringLiteral("groundwave"), QStringLiteral("regional skywave"), QStringLiteral("DX skywave") }, QStringLiteral("Best after dark; daytime range is usually limited."), QStringLiteral("Local to regional work and difficult night DX."), QStringLiteral("High atmospheric and man-made noise tendency."), QStringLiteral("Efficient antennas are large; compact antennas are compromise designs."), QStringLiteral("high variability")),
		make_profile(QStringLiteral("80m"), { QStringLiteral("groundwave"), QStringLiteral("NVIS"), QStringLiteral("regional skywave"), QStringLiteral("DX skywave") }, QStringLiteral("Daytime shorter paths; night-time regional and DX paths."), QStringLiteral("Local, regional, and night DX depending on conditions."), QStringLiteral("Often noisy, especially in summer or urban locations."), QStringLiteral("Wire antennas are practical but still physically large."), QStringLiteral("medium/high variability")),
		make_profile(QStringLiteral("75m Broadcast"), { QStringLiteral("regional skywave"), QStringLiteral("DX skywave") }, QStringLiteral("Mainly evening and night listening; daytime paths are usually shorter."), QStringLiteral("Regional and longer-distance HF broadcast reception depending on season and conditions."), QStringLiteral("High atmospheric and local noise tendency."), QStringLiteral("Receive antennas are physically long unless loaded or compromised."), QStringLiteral("high variability")),
		make_profile(QStringLiteral("60m"), { QStringLiteral("NVIS"), QStringLiteral("regional skywave") }, QStringLiteral("Often useful for regional paths across day/night changes."), QStringLiteral("Regional communication focus where permitted."), QStringLiteral("Moderate noise tendency."), QStringLiteral("Wire antennas are practical; legal availability varies."), QStringLiteral("medium variability")),
		make_profile(QStringLiteral("49m Broadcast"), { QStringLiteral("regional skywave"), QStringLiteral("DX skywave") }, QStringLiteral("Often useful in evening and night, with some regional paths at other times."), QStringLiteral("Common international shortwave broadcast listening character."), QStringLiteral("Moderate to high noise and interference tendency."), QStringLiteral("Half-wave receiving dipoles and general shortwave listening wires are practical."), QStringLiteral("medium/high variability")),
		make_profile(QStringLiteral("48m Informal"), { QStringLiteral("regional skywave"), QStringLiteral("DX skywave") }, QStringLiteral("Often evening and night dependent."), QStringLiteral("Informal listening/reference range with activity varying by country and time."), QStringLiteral("Moderate to high noise and interference tendency."), QStringLiteral("General shortwave listening antennas are practical."), QStringLiteral("high variability")),
		make_profile(QStringLiteral("40m"), { QStringLiteral("NVIS"), QStringLiteral("regional skywave"), QStringLiteral("DX skywave") }, QStringLiteral("Regional daytime paths and longer night paths."), QStringLiteral("Strong general-purpose HF band."), QStringLiteral("Moderate to high noise and congestion."), QStringLiteral("Half-wave wire antennas are practical for many sites."), QStringLiteral("medium variability")),
		make_profile(QStringLiteral("30m"), { QStringLiteral("regional skywave"), QStringLiteral("DX skywave") }, QStringLiteral("Often supports daytime and evening paths."), QStringLiteral("Regional and DX digital/CW-style use depending on rules."), QStringLiteral("Often quieter than lower HF bands."), QStringLiteral("Wire and portable antennas are practical."), QStringLiteral("medium variability")),
		make_profile(QStringLiteral("20m"), { QStringLiteral("regional skywave"), QStringLiteral("DX skywave") }, QStringLiteral("Often best in daylight and around greyline, but varies with solar conditions."), QStringLiteral("Main HF DX character when open."), QStringLiteral("Moderate noise; congestion can be high."), QStringLiteral("Dipoles, verticals, and beams are practical."), QStringLiteral("medium variability")),
		make_profile(QStringLiteral("17m"), { QStringLiteral("DX skywave") }, QStringLiteral("Often daylight and solar-condition dependent."), QStringLiteral("DX-capable when ionospheric support is present."), QStringLiteral("Usually less crowded than some larger HF bands."), QStringLiteral("Compact HF antennas are practical."), QStringLiteral("medium/high variability")),
		make_profile(QStringLiteral("15m"), { QStringLiteral("DX skywave") }, QStringLiteral("Often daylight and solar-cycle dependent."), QStringLiteral("Strong DX character when open."), QStringLiteral("Usually lower atmospheric noise than lower HF."), QStringLiteral("Small beams and wires are practical."), QStringLiteral("high variability")),
		make_profile(QStringLiteral("12m"), { QStringLiteral("DX skywave") }, QStringLiteral("Mostly opening-dependent."), QStringLiteral("Can support excellent DX during openings."), QStringLiteral("Low atmospheric noise tendency."), QStringLiteral("Compact antennas are practical."), QStringLiteral("high variability")),
		make_profile(QStringLiteral("10m"), { QStringLiteral("line-of-sight"), QStringLiteral("sporadic-E"), QStringLiteral("DX skywave") }, QStringLiteral("Highly dependent on season and solar conditions."), QStringLiteral("Local work plus dramatic condition-dependent openings."), QStringLiteral("Low atmospheric noise tendency."), QStringLiteral("Small antennas are practical."), QStringLiteral("high variability")),
		make_profile(QStringLiteral("6m"), { QStringLiteral("line-of-sight"), QStringLiteral("sporadic-E"), QStringLiteral("tropospheric") }, QStringLiteral("Openings can be seasonal and short-lived."), QStringLiteral("Local VHF plus condition-dependent longer paths."), QStringLiteral("Usually low noise but local interference matters."), QStringLiteral("Small beams and verticals are practical."), QStringLiteral("high variability")),
		make_profile(QStringLiteral("4m"), { QStringLiteral("line-of-sight"), QStringLiteral("sporadic-E"), QStringLiteral("tropospheric") }, QStringLiteral("Openings can be seasonal and allocation-dependent."), QStringLiteral("Local VHF plus condition-dependent openings."), QStringLiteral("Usually low noise but local interference matters."), QStringLiteral("Small Yagis and verticals are practical."), QStringLiteral("high variability")),
		make_profile(QStringLiteral("2m"), { QStringLiteral("line-of-sight"), QStringLiteral("tropospheric"), QStringLiteral("satellite/space") }, QStringLiteral("Mostly independent of day/night, with weather-related enhancements possible."), QStringLiteral("Local, repeater, weak-signal, satellite, and enhanced path work."), QStringLiteral("Local noise and receiver overload can matter."), QStringLiteral("Compact antennas and Yagis are practical."), QStringLiteral("medium variability")),
		make_profile(QStringLiteral("70cm"), { QStringLiteral("line-of-sight"), QStringLiteral("tropospheric"), QStringLiteral("satellite/space") }, QStringLiteral("Mostly independent of day/night."), QStringLiteral("Local, repeater, satellite, and short-range work."), QStringLiteral("Building, foliage, and terrain losses are significant."), QStringLiteral("Very compact antennas are practical."), QStringLiteral("medium variability"))
	};

	return data;
}

}

bool
propagation_profile_by_band_name(const QString &band_name, PropagationProfile &profile)
{
	for (const PropagationProfile &candidate : profiles()) {
		if (candidate.band_name.compare(band_name, Qt::CaseInsensitive) == 0) {
			profile = candidate;
			return true;
		}
	}

	return false;
}

bool
propagation_profile_by_frequency(double frequency_mhz, PropagationProfile &profile)
{
	BandReference band;

	if (!band_reference_by_frequency(frequency_mhz, band))
		return false;

	return propagation_profile_by_band_name(band.name, profile);
}

}
