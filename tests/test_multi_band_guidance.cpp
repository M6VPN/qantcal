// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_multi_band_guidance.cpp

#include "project/multi_band_guidance.h"

#include <cassert>

namespace {

qantcal::project::AntennaTarget
target(const QString &band_name, double frequency_mhz, qantcal::reference::BandService service)
{
	qantcal::project::AntennaTarget target;

	target.band_name = band_name;
	target.band_service = service;
	target.enabled = true;
	target.frequency_mhz = frequency_mhz;

	return target;
}

void
test_close_targets_warn_about_sensitivity()
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();

	project.targets.append(target(QStringLiteral("20m low"), 14.0, qantcal::reference::BandService::Amateur));
	project.targets.append(target(QStringLiteral("20m high"), 14.35, qantcal::reference::BandService::Amateur));

	const qantcal::project::MultiBandGuidance guidance =
		qantcal::project::create_multi_band_guidance(project);

	assert(guidance.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("close in frequency")));
}

void
test_harmonic_targets_include_wire_guidance()
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();

	project.antenna_type = qantcal::calculators::AntennaType::HalfWaveDipole;
	project.targets.append(target(QStringLiteral("40m"), 7.1, qantcal::reference::BandService::Amateur));
	project.targets.append(target(QStringLiteral("20m"), 14.2, qantcal::reference::BandService::Amateur));

	const QString text = qantcal::project::multi_band_guidance_text(
		qantcal::project::create_multi_band_guidance(project)
	);

	assert(text.contains(QStringLiteral("Fan dipoles")));
	assert(text.contains(QStringLiteral("Trap antennas")));
	assert(text.contains(QStringLiteral("Harmonic-like")));
	assert(text.contains(QStringLiteral("Common feedpoint")));
}

void
test_mixed_services_warn()
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();

	project.targets.append(target(QStringLiteral("40m"), 7.1, qantcal::reference::BandService::Amateur));
	project.targets.append(target(QStringLiteral("49m Broadcast"), 6.05, qantcal::reference::BandService::Broadcast));

	const qantcal::project::MultiBandGuidance guidance =
		qantcal::project::create_multi_band_guidance(project);

	assert(guidance.warnings.join(QStringLiteral("\n")).contains(QStringLiteral("mix service types")));
}

void
test_single_target_has_minimal_guidance()
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();

	project.targets.append(target(QStringLiteral("40m"), 7.1, qantcal::reference::BandService::Amateur));

	const qantcal::project::MultiBandGuidance guidance =
		qantcal::project::create_multi_band_guidance(project);

	assert(guidance.notes.join(QStringLiteral("\n")).contains(QStringLiteral("Single target")));
	assert(guidance.warnings.isEmpty());
}

void
test_yagi_multi_target_warns_about_separate_designs()
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();

	project.antenna_type = qantcal::calculators::AntennaType::Yagi;
	project.targets.append(target(QStringLiteral("2m"), 144.3, qantcal::reference::BandService::Amateur));
	project.targets.append(target(QStringLiteral("70cm"), 433.5, qantcal::reference::BandService::Amateur));

	const QString text = qantcal::project::multi_band_guidance_text(
		qantcal::project::create_multi_band_guidance(project)
	);

	assert(text.contains(QStringLiteral("separate starting design")));
	assert(text.contains(QStringLiteral("inter-band coupling")));
}

}

int
main()
{
	test_close_targets_warn_about_sensitivity();
	test_harmonic_targets_include_wire_guidance();
	test_mixed_services_warn();
	test_single_target_has_minimal_guidance();
	test_yagi_multi_target_warns_about_separate_designs();

	return 0;
}
