// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/band_reference.cpp

#include "band_reference.h"

namespace qantcal::reference {

namespace {

const QString WARNING = QStringLiteral("Convenience design reference only. Check your national regulator, licence class, and current band plan. UK users should check RSGB and Ofcom sources.");

BandReference
make_band(
	const QString &name,
	double lower_frequency_mhz,
	double upper_frequency_mhz,
	double design_frequency_mhz,
	const QString &wavelength_label,
	const QString &category,
	const QString &antenna_notes,
	const QString &propagation_notes,
	const QString &use_case_notes
)
{
	BandReference reference;

	reference.name = name;
	reference.lower_frequency_mhz = lower_frequency_mhz;
	reference.upper_frequency_mhz = upper_frequency_mhz;
	reference.design_frequency_mhz = design_frequency_mhz;
	reference.wavelength_label = wavelength_label;
	reference.category = category;
	reference.antenna_notes = antenna_notes;
	reference.propagation_notes = propagation_notes;
	reference.use_case_notes = use_case_notes;
	reference.warning = WARNING;

	return reference;
}

}

const QVector<BandReference> &
band_references()
{
	static const QVector<BandReference> bands = {
		make_band(QStringLiteral("160m"), 1.800, 2.000, 1.900, QStringLiteral("160 metres"), QStringLiteral("MF"), QStringLiteral("Large antennas, loaded verticals, inverted Ls, and compact compromises are common."), QStringLiteral("Mostly night-time regional and DX skywave with high noise and ground losses."), QStringLiteral("Top-band operating is installation-sensitive and often noise-limited.")),
		make_band(QStringLiteral("80m"), 3.500, 4.000, 3.650, QStringLiteral("80 metres"), QStringLiteral("HF"), QStringLiteral("Dipoles, inverted Vees, verticals, loops, and loaded wires are common."), QStringLiteral("Night-time regional and DX skywave, with daytime shorter-range use."), QStringLiteral("Useful for local to regional contacts and night DX when conditions allow.")),
		make_band(QStringLiteral("60m"), 5.000, 5.500, 5.350, QStringLiteral("60 metres"), QStringLiteral("HF"), QStringLiteral("Wires and compact dipoles are practical, but allocations vary strongly by country."), QStringLiteral("Often useful for NVIS and regional skywave."), QStringLiteral("Treat as highly licence-dependent and channel/band-plan specific.")),
		make_band(QStringLiteral("40m"), 7.000, 7.300, 7.100, QStringLiteral("40 metres"), QStringLiteral("HF"), QStringLiteral("Half-wave dipoles, inverted Vees, verticals, and loops are practical."), QStringLiteral("Regional daytime paths and longer night-time skywave are common."), QStringLiteral("A practical first HF antenna band, but noise and crowding vary.")),
		make_band(QStringLiteral("30m"), 10.100, 10.150, 10.125, QStringLiteral("30 metres"), QStringLiteral("HF"), QStringLiteral("Wire dipoles, verticals, and compact portable antennas are common."), QStringLiteral("Often supports regional and DX paths with lower noise than lower HF bands."), QStringLiteral("Mode and licence restrictions vary, so check current rules.")),
		make_band(QStringLiteral("20m"), 14.000, 14.350, 14.200, QStringLiteral("20 metres"), QStringLiteral("HF"), QStringLiteral("Dipoles, verticals, beams, and portable antennas are common."), QStringLiteral("Often a main daytime DX band, with paths varying by season and solar conditions."), QStringLiteral("Good general DX band when ionospheric conditions support it.")),
		make_band(QStringLiteral("17m"), 18.068, 18.168, 18.118, QStringLiteral("17 metres"), QStringLiteral("HF"), QStringLiteral("Dipoles, verticals, and small beams are practical."), QStringLiteral("Can support DX with less congestion than larger contest bands."), QStringLiteral("Propagation is solar-cycle and time-of-day dependent.")),
		make_band(QStringLiteral("15m"), 21.000, 21.450, 21.225, QStringLiteral("15 metres"), QStringLiteral("HF"), QStringLiteral("Wire antennas, verticals, and compact beams are practical."), QStringLiteral("DX-friendly when the band is open, especially with stronger solar conditions."), QStringLiteral("Can sound dead when the MUF is too low.")),
		make_band(QStringLiteral("12m"), 24.890, 24.990, 24.940, QStringLiteral("12 metres"), QStringLiteral("HF"), QStringLiteral("Compact dipoles, verticals, and small beams are practical."), QStringLiteral("Often condition-dependent with good DX during openings."), QStringLiteral("Openings can be excellent but irregular.")),
		make_band(QStringLiteral("10m"), 28.000, 29.700, 28.500, QStringLiteral("10 metres"), QStringLiteral("HF"), QStringLiteral("Small antennas and beams are practical."), QStringLiteral("Local groundwave, sporadic-E, and solar-cycle DX openings are possible."), QStringLiteral("Long-distance openings are highly condition-dependent.")),
		make_band(QStringLiteral("6m"), 50.000, 54.000, 50.200, QStringLiteral("6 metres"), QStringLiteral("VHF"), QStringLiteral("Small beams, verticals, and loops are practical."), QStringLiteral("Line-of-sight, sporadic-E, tropospheric, and rare DX modes are possible."), QStringLiteral("Longer paths are highly condition-dependent.")),
		make_band(QStringLiteral("4m"), 70.000, 70.500, 70.200, QStringLiteral("4 metres"), QStringLiteral("VHF"), QStringLiteral("Small Yagis and verticals are practical where the band is allocated."), QStringLiteral("Line-of-sight, sporadic-E, and tropospheric paths may occur."), QStringLiteral("Allocation is not worldwide, so check local rules carefully.")),
		make_band(QStringLiteral("2m"), 144.000, 148.000, 144.300, QStringLiteral("2 metres"), QStringLiteral("VHF"), QStringLiteral("Verticals, small Yagis, and portable antennas are practical."), QStringLiteral("Mostly line-of-sight, with tropospheric, satellite, and occasional enhanced paths."), QStringLiteral("Height, terrain, antenna gain, and local noise dominate practical range.")),
		make_band(QStringLiteral("70cm"), 430.000, 440.000, 433.500, QStringLiteral("70 centimetres"), QStringLiteral("UHF"), QStringLiteral("Compact verticals, handheld antennas, and small Yagis are practical."), QStringLiteral("Mostly line-of-sight with strong terrain, building, and foliage effects."), QStringLiteral("Useful for local, repeater, satellite, and short-range work where allocated."))
	};

	return bands;
}

bool
band_reference_by_frequency(double frequency_mhz, BandReference &reference)
{
	for (const BandReference &band : band_references()) {
		if (frequency_mhz >= band.lower_frequency_mhz && frequency_mhz <= band.upper_frequency_mhz) {
			reference = band;
			return true;
		}
	}

	return false;
}

bool
band_reference_by_name(const QString &name, BandReference &reference)
{
	for (const BandReference &band : band_references()) {
		if (band.name.compare(name, Qt::CaseInsensitive) == 0) {
			reference = band;
			return true;
		}
	}

	return false;
}

QString
band_reference_warning()
{
	return WARNING;
}

}
