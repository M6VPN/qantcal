// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/reference/mode_reference.h

#ifndef QANTCAL_REFERENCE_MODE_REFERENCE_H
#define QANTCAL_REFERENCE_MODE_REFERENCE_H

#include <QString>
#include <QVector>

namespace qantcal::reference {

enum class ModeType {
	FmVoice,
	AmVoice,
	SsbVoice,
	Cw,
	DigitalData
};

struct ModeReference {
	ModeType mode = ModeType::SsbVoice;
	QString key;
	QString name;
	QString bandwidth_category;
	QString use_notes;
	QString weak_signal_notes;
	QString beginner_notes;
	QString caution;
};

const QVector<ModeReference> &mode_references();
bool mode_reference_by_key(const QString &key, ModeReference &reference);
QString mode_type_key(ModeType mode);
QString mode_type_label(ModeType mode);
ModeType mode_type_from_key(const QString &key);

}

#endif
