// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/mode_reference.cpp

#include "mode_reference.h"

namespace qantcal::reference {

namespace {

ModeReference
make_mode(
	ModeType mode,
	const QString &key,
	const QString &name,
	const QString &bandwidth_category,
	const QString &use_notes,
	const QString &weak_signal_notes,
	const QString &beginner_notes
)
{
	ModeReference reference;

	reference.mode = mode;
	reference.key = key;
	reference.name = name;
	reference.bandwidth_category = bandwidth_category;
	reference.use_notes = use_notes;
	reference.weak_signal_notes = weak_signal_notes;
	reference.beginner_notes = beginner_notes;
	reference.caution = QStringLiteral("Actual performance depends on power, bandwidth, coding, receiver, noise, antennas, and conditions.");

	return reference;
}

}

const QVector<ModeReference> &
mode_references()
{
	static const QVector<ModeReference> modes = {
		make_mode(ModeType::FmVoice, QStringLiteral("fm_voice"), QStringLiteral("FM voice"), QStringLiteral("wide voice"), QStringLiteral("Common on VHF/UHF local, repeater, and handheld work."), QStringLiteral("Less suited to weak-signal edge-of-noise work than narrower modes."), QStringLiteral("Simple to operate, but range is often limited by line-of-sight, terrain, and repeaters.")),
		make_mode(ModeType::AmVoice, QStringLiteral("am_voice"), QStringLiteral("AM voice"), QStringLiteral("wide voice"), QStringLiteral("Used on some HF and VHF activity by convention and interest."), QStringLiteral("Usually less power-efficient for weak-signal work than SSB or CW."), QStringLiteral("Check band-plan practice and transmitter bandwidth before use.")),
		make_mode(ModeType::SsbVoice, QStringLiteral("ssb_voice"), QStringLiteral("SSB voice"), QStringLiteral("narrow voice"), QStringLiteral("Common for HF and VHF weak-signal voice operation."), QStringLiteral("Generally practical for weaker signals than FM voice."), QStringLiteral("Requires tuning care and suitable band/mode practice.")),
		make_mode(ModeType::Cw, QStringLiteral("cw"), QStringLiteral("CW"), QStringLiteral("very narrow"), QStringLiteral("Used for low-bandwidth manual telegraphy and weak-signal contacts."), QStringLiteral("Often effective under weak-signal conditions, depending on operator skill and receiver filtering."), QStringLiteral("Requires learning Morse code and checking local band-plan practice.")),
		make_mode(ModeType::DigitalData, QStringLiteral("digital_data"), QStringLiteral("Digital/data"), QStringLiteral("varies by mode"), QStringLiteral("Covers many keyboard, weak-signal, image, and telemetry-style modes."), QStringLiteral("Some digital modes can work well at low signal levels, but behaviour is mode-specific."), QStringLiteral("Use the correct software, timing, audio levels, and occupied bandwidth for the chosen mode."))
	};

	return modes;
}

bool
mode_reference_by_key(const QString &key, ModeReference &reference)
{
	for (const ModeReference &mode : mode_references()) {
		if (mode.key == key) {
			reference = mode;
			return true;
		}
	}

	return false;
}

QString
mode_type_key(ModeType mode)
{
	switch (mode) {
	case ModeType::FmVoice:
		return QStringLiteral("fm_voice");
	case ModeType::AmVoice:
		return QStringLiteral("am_voice");
	case ModeType::SsbVoice:
		return QStringLiteral("ssb_voice");
	case ModeType::Cw:
		return QStringLiteral("cw");
	case ModeType::DigitalData:
		return QStringLiteral("digital_data");
	}

	return QStringLiteral("ssb_voice");
}

QString
mode_type_label(ModeType mode)
{
	ModeReference reference;

	if (mode_reference_by_key(mode_type_key(mode), reference))
		return reference.name;

	return QStringLiteral("SSB voice");
}

ModeType
mode_type_from_key(const QString &key)
{
	for (const ModeReference &mode : mode_references()) {
		if (mode.key == key)
			return mode.mode;
	}

	return ModeType::SsbVoice;
}

}
