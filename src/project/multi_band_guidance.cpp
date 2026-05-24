// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/multi_band_guidance.cpp

#include "multi_band_guidance.h"

#include <algorithm>
#include <cmath>

namespace qantcal::project {

namespace {

QVector<AntennaTarget>
enabled_targets(const QVector<AntennaTarget> &targets)
{
	QVector<AntennaTarget> enabled;

	for (const AntennaTarget &target : targets) {
		if (target.enabled && target.frequency_mhz > 0.0)
			enabled.append(target);
	}
	std::sort(enabled.begin(), enabled.end(), [](const AntennaTarget &left, const AntennaTarget &right) {
		return left.frequency_mhz < right.frequency_mhz;
	});

	return enabled;
}

bool
has_close_targets(const QVector<AntennaTarget> &targets)
{
	for (int i = 1; i < targets.size(); ++i) {
		const double previous = targets[i - 1].frequency_mhz;
		const double current = targets[i].frequency_mhz;

		if (std::fabs(current - previous) / previous <= 0.08)
			return true;
	}

	return false;
}

bool
has_harmonic_like_targets(const QVector<AntennaTarget> &targets)
{
	for (int i = 0; i < targets.size(); ++i) {
		for (int j = i + 1; j < targets.size(); ++j) {
			const double ratio = targets[j].frequency_mhz / targets[i].frequency_mhz;
			const double rounded = std::round(ratio);

			if (rounded >= 2.0 && rounded <= 5.0 && std::fabs(ratio - rounded) <= 0.08)
				return true;
		}
	}

	return false;
}

bool
has_mixed_services(const QVector<AntennaTarget> &targets)
{
	reference::BandService service = reference::BandService::Unknown;

	for (const AntennaTarget &target : targets) {
		if (target.band_service == reference::BandService::Unknown)
			continue;
		if (service == reference::BandService::Unknown) {
			service = target.band_service;
			continue;
		}
		if (target.band_service != service)
			return true;
	}

	return false;
}

bool
is_wire_antenna(calculators::AntennaType antenna_type)
{
	return antenna_type == calculators::AntennaType::HalfWaveDipole
		|| antenna_type == calculators::AntennaType::FoldedDipole
		|| antenna_type == calculators::AntennaType::Halo
		|| antenna_type == calculators::AntennaType::InvertedVee
		|| antenna_type == calculators::AntennaType::EndFedHalfWave
		|| antenna_type == calculators::AntennaType::FullWaveLoop
		|| antenna_type == calculators::AntennaType::RandomWire;
}

}

MultiBandGuidance
create_multi_band_guidance(const AntennaProject &project)
{
	const QVector<AntennaTarget> targets = enabled_targets(project.targets);
	MultiBandGuidance guidance;

	if (targets.isEmpty()) {
		guidance.notes << QStringLiteral("No enabled target bands are saved for this project.");
		return guidance;
	}
	if (targets.size() == 1) {
		guidance.notes << QStringLiteral("Single target saved. Multi-band interaction guidance is not needed.");
		return guidance;
	}

	guidance.notes << QStringLiteral("Targets are calculated independently. Physical interaction, shared feedpoint impedance, and final trim are not modelled.");
	guidance.warnings << QStringLiteral("Common feedpoint impedance can change when multiple elements, bands, traps, or stubs are connected together.");
	guidance.warnings << QStringLiteral("Use an analyser, VNA, or low-power SWR checks on each band before normal operation.");

	if (project.antenna_type == calculators::AntennaType::Yagi) {
		guidance.warnings << QStringLiteral("Each saved Yagi target is a separate starting design, not one broadband or multi-band Yagi.");
		guidance.notes << QStringLiteral("Yagi element spacing, matching, traps, and inter-band coupling need modelling or measurement before construction.");
		return guidance;
	}

	if (is_wire_antenna(project.antenna_type)) {
		guidance.notes << QStringLiteral("Fan dipoles need physical spacing between legs, strain relief at the feedpoint, and trim checks from the lowest band upward.");
		guidance.notes << QStringLiteral("Trap antennas need measured trap resonance, voltage and current margin, weatherproofing, and element length adjustment after traps are installed.");
	}
	if (has_harmonic_like_targets(targets))
		guidance.notes << QStringLiteral("Harmonic-like target bands may share useful wire lengths, but feed impedance and current distribution still need measurement.");
	if (has_close_targets(targets))
		guidance.warnings << QStringLiteral("Some targets are close in frequency, so element interaction and narrow trim changes may be more sensitive.");
	if (has_mixed_services(targets))
		guidance.warnings << QStringLiteral("Saved targets mix service types. Confirm receive-only and transmit-authorised bands separately.");

	return guidance;
}

QString
multi_band_guidance_text(const MultiBandGuidance &guidance)
{
	QStringList lines;

	if (!guidance.notes.isEmpty()) {
		lines << QStringLiteral("Notes:");
		lines << guidance.notes;
	}
	if (!guidance.warnings.isEmpty()) {
		if (!lines.isEmpty())
			lines << QString();
		lines << QStringLiteral("Warnings:");
		lines << guidance.warnings;
	}

	return lines.join(QStringLiteral("\n"));
}

}
