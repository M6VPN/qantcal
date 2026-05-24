// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/antenna_project.cpp

#include "antenna_project.h"

#include <QDateTime>

namespace qantcal::project {

AntennaProject
default_project()
{
	AntennaProject project;

	project.created_utc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
	project.updated_utc = project.created_utc;

	return project;
}

QString
antenna_type_to_key(calculators::AntennaType antenna_type)
{
	switch (antenna_type) {
	case calculators::AntennaType::HalfWaveDipole:
		return QStringLiteral("half_wave_dipole");
	case calculators::AntennaType::FoldedDipole:
		return QStringLiteral("folded_dipole");
	case calculators::AntennaType::QuarterWaveVertical:
		return QStringLiteral("quarter_wave_vertical");
	case calculators::AntennaType::EndFedHalfWave:
		return QStringLiteral("end_fed_half_wave");
	case calculators::AntennaType::FullWaveLoop:
		return QStringLiteral("full_wave_loop");
	case calculators::AntennaType::InvertedVee:
		return QStringLiteral("inverted_vee");
	case calculators::AntennaType::RandomWire:
		return QStringLiteral("random_wire");
	case calculators::AntennaType::Yagi:
		return QStringLiteral("yagi");
	}

	return QStringLiteral("half_wave_dipole");
}

calculators::AntennaType
antenna_type_from_key(const QString &key)
{
	if (key == QStringLiteral("folded_dipole"))
		return calculators::AntennaType::FoldedDipole;
	if (key == QStringLiteral("quarter_wave_vertical"))
		return calculators::AntennaType::QuarterWaveVertical;
	if (key == QStringLiteral("end_fed_half_wave"))
		return calculators::AntennaType::EndFedHalfWave;
	if (key == QStringLiteral("full_wave_loop"))
		return calculators::AntennaType::FullWaveLoop;
	if (key == QStringLiteral("inverted_vee"))
		return calculators::AntennaType::InvertedVee;
	if (key == QStringLiteral("random_wire"))
		return calculators::AntennaType::RandomWire;
	if (key == QStringLiteral("yagi"))
		return calculators::AntennaType::Yagi;

	return calculators::AntennaType::HalfWaveDipole;
}

}
