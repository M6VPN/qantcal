// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/broadcast_band_reference.cpp

#include "broadcast_band_reference.h"

namespace qantcal::reference {

namespace {

const QString BROADCAST_WARNING = QStringLiteral("Shortwave broadcast/reference band. These dimensions are suitable as starting points for receive antennas or legally authorised transmission only. qantcal does not grant authority to transmit.");

BandReference
make_broadcast_band(
	const QString &name,
	BandService service,
	double lower_frequency_mhz,
	double upper_frequency_mhz,
	double design_frequency_mhz,
	const QString &wavelength_label,
	const QString &mode_notes,
	const QString &antenna_notes,
	const QString &propagation_notes,
	const QString &use_case_notes
)
{
	BandReference reference;

	reference.name = name;
	reference.service = service;
	reference.lower_frequency_mhz = lower_frequency_mhz;
	reference.upper_frequency_mhz = upper_frequency_mhz;
	reference.design_frequency_mhz = design_frequency_mhz;
	reference.wavelength_label = wavelength_label;
	reference.category = QStringLiteral("HF");
	reference.mode_notes = mode_notes;
	reference.antenna_notes = antenna_notes;
	reference.propagation_notes = propagation_notes;
	reference.use_case_notes = use_case_notes;
	reference.warning = BROADCAST_WARNING;

	return reference;
}

}

const QVector<BandReference> &
broadcast_band_references()
{
	static const QVector<BandReference> bands = {
		make_broadcast_band(
			QStringLiteral("75m Broadcast"),
			BandService::Broadcast,
			3.900,
			4.000,
			3.950,
			QStringLiteral("75 metres"),
			QStringLiteral("AM broadcast listening and legally authorised broadcast operation where permitted."),
			QStringLiteral("Long wire antennas, dipoles, inverted Vees, and loops are common receive starting points."),
			QStringLiteral("Night/regional/DX HF broadcast band with high noise tendency."),
			QStringLiteral("Not an amateur allocation in many regions; check national allocation and licensing data.")
		),
		make_broadcast_band(
			QStringLiteral("49m Broadcast"),
			BandService::Broadcast,
			5.900,
			6.200,
			6.050,
			QStringLiteral("49 metres"),
			QStringLiteral("Common international shortwave broadcast listening band."),
			QStringLiteral("Receive dipoles, inverted Vees, EFHW wires, loops, and random-wire receive antennas are practical. A half-wave dipole is roughly 24-25 m before shortening."),
			QStringLiteral("Often useful evening/night and for regional or DX broadcast paths depending on conditions."),
			QStringLiteral("Conventional shortwave broadcast band; transmitting requires proper authority.")
		),
		make_broadcast_band(
			QStringLiteral("48m Informal"),
			BandService::Informal,
			6.200,
			6.500,
			6.300,
			QStringLiteral("48 metres"),
			QStringLiteral("Informal/free-radio/utility listening reference; not a standard ITU international broadcast band entry."),
			QStringLiteral("Receive wires, loops, and general shortwave listening antennas are practical starting points."),
			QStringLiteral("Similar lower-HF evening/night skywave character, with use varying by region and activity."),
			QStringLiteral("Legal status depends on jurisdiction and use; this entry is a listening/reference aid only.")
		)
	};

	return bands;
}

}
