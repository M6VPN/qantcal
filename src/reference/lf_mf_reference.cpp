// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/lf_mf_reference.cpp

#include "lf_mf_reference.h"

namespace qantcal::reference {

namespace {

const QString AMATEUR_WARNING = QStringLiteral("LF/MF amateur reference only. Check national regulator, licence terms, band plan, notification requirements, and permitted ERP/EIRP before transmitting.");
const QString BROADCAST_WARNING = QStringLiteral("LW/MW broadcast reference/listening band. These dimensions are suitable as starting points for receive antennas or legally authorised transmission only. qantcal does not grant authority to transmit.");
const QString EXPERIMENTAL_WARNING = QStringLiteral("Experimental/reference range only. Do not treat this as an amateur band or authority to transmit. Check current national rules before any operation.");

BandReference
make_lf_mf_band(
	const QString &name,
	BandService service,
	double lower_frequency_mhz,
	double upper_frequency_mhz,
	double design_frequency_mhz,
	const QString &wavelength_label,
	const QString &category,
	const QString &mode_notes,
	const QString &antenna_notes,
	const QString &propagation_notes,
	const QString &use_case_notes,
	const QString &warning
)
{
	BandReference reference;

	reference.name = name;
	reference.service = service;
	reference.lower_frequency_mhz = lower_frequency_mhz;
	reference.upper_frequency_mhz = upper_frequency_mhz;
	reference.design_frequency_mhz = design_frequency_mhz;
	reference.wavelength_label = wavelength_label;
	reference.category = category;
	reference.mode_notes = mode_notes;
	reference.antenna_notes = antenna_notes;
	reference.propagation_notes = propagation_notes;
	reference.use_case_notes = use_case_notes;
	reference.warning = warning;

	return reference;
}

}

const QVector<BandReference> &
lf_mf_band_references()
{
	static const QVector<BandReference> bands = {
		make_lf_mf_band(
			QStringLiteral("2200m Amateur"),
			BandService::Amateur,
			0.1357,
			0.1378,
			0.1365,
			QStringLiteral("2200 metres"),
			QStringLiteral("LF"),
			QStringLiteral("Narrow-band amateur LF operation where authorised; allocations and power limits vary by country."),
			QStringLiteral("Full-size antennas are huge. Practical transmit antennas are usually electrically short vertical, inverted-L, or top-loaded systems with loading coils and substantial ground/counterpoise."),
			QStringLiteral("Groundwave and weak-signal LF paths dominate; noise, ground loss, and permitted radiated power are major limits."),
			QStringLiteral("Secondary allocation in many countries with very low permitted radiated power and notification or siting rules."),
			AMATEUR_WARNING
		),
		make_lf_mf_band(
			QStringLiteral("630m Amateur"),
			BandService::Amateur,
			0.472,
			0.479,
			0.475,
			QStringLiteral("630 metres"),
			QStringLiteral("MF"),
			QStringLiteral("Narrow-band amateur MF operation where authorised; allocations and power limits vary by country."),
			QStringLiteral("Short verticals, inverted-L antennas, and T/top-loaded verticals often need loading coils and substantial ground/counterpoise systems."),
			QStringLiteral("Groundwave and night skywave are possible; ground losses and local noise strongly affect results."),
			QStringLiteral("Secondary allocation in many countries. Check local power, notification, and separation requirements."),
			AMATEUR_WARNING
		),
		make_lf_mf_band(
			QStringLiteral("LW Broadcast"),
			BandService::Broadcast,
			0.1485,
			0.2835,
			0.198,
			QStringLiteral("longwave"),
			QStringLiteral("LF"),
			QStringLiteral("AM broadcast listening and legally authorised broadcast operation where permitted."),
			QStringLiteral("Receive antennas can be compact active probes, loops, or long wires. Transmit antennas require specialist design and legal authority."),
			QStringLiteral("Region-dependent longwave broadcast reception with strong noise and groundwave character."),
			QStringLiteral("Broadcast reference/listening band; not an amateur transmit band."),
			BROADCAST_WARNING
		),
		make_lf_mf_band(
			QStringLiteral("MW Broadcast Region 1/3"),
			BandService::Broadcast,
			0.5265,
			1.6065,
			1.000,
			QStringLiteral("medium wave"),
			QStringLiteral("MF"),
			QStringLiteral("AM broadcast listening and legally authorised broadcast operation where permitted."),
			QStringLiteral("Receive loops, ferrite rods, active probes, and long wires are practical. Broadcast transmit antennas require professional design."),
			QStringLiteral("Medium-wave groundwave and night skywave vary strongly with location and interference."),
			QStringLiteral("Region 1/3 style medium-wave broadcast reference; not an amateur transmit band."),
			BROADCAST_WARNING
		),
		make_lf_mf_band(
			QStringLiteral("MW Broadcast Americas"),
			BandService::Broadcast,
			0.525,
			1.705,
			1.000,
			QStringLiteral("medium wave"),
			QStringLiteral("MF"),
			QStringLiteral("AM broadcast listening and legally authorised broadcast operation where permitted."),
			QStringLiteral("Receive loops, ferrite rods, active probes, and long wires are practical. Broadcast transmit antennas require professional design."),
			QStringLiteral("Medium-wave groundwave and night skywave vary strongly with location and interference."),
			QStringLiteral("Americas-style medium-wave broadcast reference; not an amateur transmit band."),
			BROADCAST_WARNING
		),
		make_lf_mf_band(
			QStringLiteral("1750m LowFER / experimental reference"),
			BandService::Experimental,
			0.160,
			0.190,
			0.175,
			QStringLiteral("1750 metres"),
			QStringLiteral("LF"),
			QStringLiteral("Experimental or low-frequency reference use where lawful."),
			QStringLiteral("Receive/reference antennas only unless the user has explicit legal authority. Electrically short antennas and loading are expected."),
			QStringLiteral("LF groundwave and noise-limited reception; practical results are highly site-dependent."),
			QStringLiteral("Reference/legal-check-required range. Not listed as an amateur band."),
			EXPERIMENTAL_WARNING
		)
	};

	return bands;
}

}
