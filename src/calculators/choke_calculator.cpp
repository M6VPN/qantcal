// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/calculators/choke_calculator.cpp

#include "choke_calculator.h"

#include <cmath>
#include <limits>

namespace qantcal::calculators {

namespace {

bool
is_positive(double value)
{
	return std::isfinite(value) && value > 0.0;
}

ChokeCalculationResult
invalid_result(const QString &error_message)
{
	ChokeCalculationResult result;

	result.error_message = error_message;

	return result;
}

QString
mix_note(ChokeCoreMix mix)
{
	switch (mix) {
	case ChokeCoreMix::Mix31:
		return QStringLiteral("Mix 31 is often used for lower-HF common-mode choking. Verify the selected core and winding with manufacturer data or measurement.");
	case ChokeCoreMix::Mix43:
		return QStringLiteral("Mix 43 is a common broadband HF/VHF choke material. Verify impedance at the actual frequency and winding layout.");
	case ChokeCoreMix::Mix61:
		return QStringLiteral("Mix 61 is usually chosen for higher-HF and VHF work. Check measured impedance before relying on it at lower HF.");
	case ChokeCoreMix::AirCore:
		return QStringLiteral("Air-core coax coils are strongly affected by diameter, spacing, cable type, and nearby objects. Treat this as planning guidance only.");
	case ChokeCoreMix::Custom:
		break;
	}

	return QStringLiteral("Use measured or datasheet impedance for the same core, cable, winding style, and frequency.");
}

void
add_warnings(ChokeCalculationResult &result, const ChokeCalculationInput &input)
{
	if (result.estimated_impedance_ohms < input.target_impedance_ohms)
		result.warnings << QStringLiteral("Estimated choking impedance is below the target; add turns, choose a different core, stack cores, or verify with measured data.");
	if (input.turns >= 8.0)
		result.warnings << QStringLiteral("High turn counts can add stray capacitance and move the choke away from the simple turns-squared estimate.");
	if (result.suggested_turns >= 8)
		result.warnings << QStringLiteral("The suggested turn count is high enough that winding layout and self-capacitance need measurement.");
	if (input.reference_impedance_ohms < 10.0)
		result.warnings << QStringLiteral("Reference impedance is low; the required choke may need a different core, stacked cores, or measured redesign.");
	if (input.mix == ChokeCoreMix::AirCore)
		result.warnings << QStringLiteral("Air-core choke impedance is not ferrite impedance; measure the finished choke before using it for common-mode control.");
}

}

ChokeCalculationResult
calculate_choke(const ChokeCalculationInput &input)
{
	if (!is_positive(input.frequency_mhz))
		return invalid_result(QStringLiteral("Frequency must be greater than zero."));
	if (!is_positive(input.reference_impedance_ohms))
		return invalid_result(QStringLiteral("Reference impedance must be greater than zero."));
	if (!is_positive(input.target_impedance_ohms))
		return invalid_result(QStringLiteral("Target impedance must be greater than zero."));
	if (!is_positive(input.turns))
		return invalid_result(QStringLiteral("Turns must be greater than zero."));

	const double suggested_turns = std::ceil(std::sqrt(input.target_impedance_ohms / input.reference_impedance_ohms));
	if (!std::isfinite(suggested_turns) || suggested_turns > static_cast<double>(std::numeric_limits<int>::max()))
		return invalid_result(QStringLiteral("Suggested turn count is outside the supported range."));

	ChokeCalculationResult result;

	result.ok = true;
	result.estimated_impedance_ohms = input.reference_impedance_ohms * input.turns * input.turns;
	result.target_ratio = result.estimated_impedance_ohms / input.target_impedance_ohms;
	result.suggested_turns = static_cast<int>(suggested_turns);
	result.mix_note = mix_note(input.mix);
	result.note = QStringLiteral("First-pass RF choke helper. It uses user-supplied measured or datasheet reference impedance and a simple turns-squared estimate. Real common-mode impedance depends on ferrite material, core geometry, cable, winding layout, frequency, heating, and measurement setup.");
	add_warnings(result, input);

	return result;
}

QString
choke_mix_label(ChokeCoreMix mix)
{
	switch (mix) {
	case ChokeCoreMix::Mix31:
		return QStringLiteral("Mix 31");
	case ChokeCoreMix::Mix43:
		return QStringLiteral("Mix 43");
	case ChokeCoreMix::Mix61:
		return QStringLiteral("Mix 61");
	case ChokeCoreMix::AirCore:
		return QStringLiteral("Air-core/coax coil");
	case ChokeCoreMix::Custom:
		break;
	}

	return QStringLiteral("Custom/unknown");
}

}
