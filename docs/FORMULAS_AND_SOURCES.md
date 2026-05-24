# Formulas and Sources

qantcal uses simple first-pass RF formulas for starting dimensions. Antenna calculators give starting dimensions only, not guaranteed final cut lengths. Real installations vary due to ground, height, nearby metal, insulation, wire diameter, bends, end effects, baluns, loading coils, traps, radials, matching networks, and surroundings.

Use an antenna analyser, VNA, SWR meter, or modelling tool to verify and trim a real antenna. Cutting long and trimming gradually is usually safer than cutting to the calculated value as a final dimension.

All calculations use metres internally. Millimetres, centimetres, metres, and feet/inches are display and input preferences only.

Invalid inputs are rejected. Impractical but mathematically valid designs still calculate and include warnings, so users can see why a simple formula may not describe a buildable antenna. These warnings are practical guidance only, not legal, safety, engineering, or compliance certification.

## Exact Physics Constants

The speed of light in vacuum is exact by SI definition:

```text
c = 299792458.0 m/s
```

qantcal uses this value internally.

## Ideal Free-Space Formulas

Frequency is converted from MHz to Hz:

```text
frequency_hz = frequency_mhz * 1000000.0
```

Wavelength:

```text
wavelength_m = 299792458.0 / frequency_hz
```

Half-wave free-space length:

```text
half_wave_m = wavelength_m / 2
```

Quarter-wave free-space length:

```text
quarter_wave_m = wavelength_m / 4
```

Full-wave loop free-space circumference:

```text
loop_m = wavelength_m
```

## Empirical Shortening and Velocity Factors

Wire antennas are usually shorter than ideal free-space calculations suggest. qantcal applies a configurable shortening factor, defaulting to:

```text
wire_factor = 0.95
```

This value is an approximation only. The best value depends on wire diameter, insulation, antenna shape, nearby objects, height, feed method, and end effects.

The application accepts user-supplied shortening factors from 0.50 to 1.00 for this first pass.

## Practicality Warnings

qantcal uses conservative first-pass warning thresholds:

- 80 m or longer: large installation area and substantial supports are likely needed.
- 250 m or longer: impractical for ordinary sites.
- 1000 m or longer: effectively impossible for typical amateur construction.
- 0.05 m or shorter: construction tolerance and connector geometry can dominate.
- Below 0.1 MHz: LF/VLF wire dimensions are extreme; LF/MF loaded or receive-only guidance is more appropriate.
- At or above 1300 MHz: simple wire formulas are poor guidance for microwave-style construction and feed geometry.

These warnings do not block calculation. They flag designs where the result is useful as a reference, not as an ordinary build plan.

## First-Pass Antenna Calculations

Half-wave dipole:

```text
total_length_m = (wavelength_m / 2) * factor
leg_length_m = total_length_m / 2
```

Folded dipole:

```text
span_m = (wavelength_m / 2) * factor
leg_length_m = span_m / 2
estimated_total_conductor_m = span_m * 2
```

The folded dipole uses the same first-pass electrical span as a half-wave dipole. A two-wire folded dipole needs roughly twice that span in conductor material because it is a folded loop. It is often near 4 times the impedance of a simple dipole, but qantcal does not calculate feed impedance, conductor spacing effects, transformation ratio, or balun design.

Halo:

```text
conductor_length_m = (wavelength_m / 2) * factor
diameter_reference_m = conductor_length_m / pi
end_gap_starting_point_m = wavelength_m * 0.015
```

A halo is treated as a half-wave dipole bent into a near-circle with a small gap opposite the feed point. The diameter and gap are first-pass construction references only. Practical halo tuning depends on conductor diameter, gap capacitance, matching method, feedline choking, mounting, nearby objects, and final trimming.

Inverted Vee:

```text
total_length_m = (wavelength_m / 2) * factor
leg_length_m = total_length_m / 2
```

The inverted Vee uses the same starting electrical length as a dipole. Apex angle, height, and end effects alter real tuning.

End-fed half-wave:

```text
wire_length_m = (wavelength_m / 2) * factor
```

Quarter-wave vertical:

```text
radiator_length_m = (wavelength_m / 4) * factor
```

Radials or a counterpoise are required, but this scaffold does not calculate radial layout in detail yet.

Full-wave loop:

```text
circumference_m = wavelength_m * factor
square_or_diamond_side_m = circumference_m / 4
```

Random wire:

```text
frequency mode: show wavelength context only
length mode: show supplied physical wire length and broad reference frequency context
```

Random-wire antennas are not presented as resonant cut lengths. qantcal gives advisory output only: tuner or matching-network guidance, counterpoise/return-path notes, and warnings where the supplied length is close to a half-wave multiple at a reference frequency. Useful real guidance still depends on the tuner, feedline, counterpoise, installation geometry, height, nearby objects, and measured behaviour.

## First-Pass Yagi Starting Dimensions

The Yagi designer produces empirical starting dimensions only. It is not a NEC, Method-of-Moments, gain, front-to-back, SWR, or impedance optimiser.

Element roles:

- reflector: a slightly longer parasitic element behind the driven element
- driven element: the fed element
- directors: shorter parasitic elements in the forward direction

Base values:

```text
wavelength_m = 299792458.0 / frequency_hz
base_half_wave_m = (wavelength_m / 2.0) * element_shortening_factor
```

The Yagi element shortening factor is accepted from 0.85 to 1.00. The default is 0.95.

Conservative preset:

```text
reflector_length_m = base_half_wave_m * 1.05
driven_length_m = base_half_wave_m
first_director_length_m = base_half_wave_m * 0.95
later_director_length_m = max(previous_director_length_m * 0.995, base_half_wave_m * 0.90)
reflector_to_driven_spacing_m = wavelength_m * 0.20
driven_to_first_director_spacing_m = wavelength_m * 0.15
director_to_director_spacing_m = wavelength_m * 0.20
```

Compact preset:

```text
reflector_to_driven_spacing_m = wavelength_m * 0.15
driven_to_first_director_spacing_m = wavelength_m * 0.12
director_to_director_spacing_m = wavelength_m * 0.15
```

The compact preset uses the same element length rules as the conservative preset. Compact Yagis are more sensitive to tuning and matching.

Long boom preset:

```text
reflector_length_m = base_half_wave_m * 1.05
driven_length_m = base_half_wave_m
first_director_length_m = base_half_wave_m * 0.94
later_director_length_m = max(previous_director_length_m * 0.995, base_half_wave_m * 0.88)
reflector_to_driven_spacing_m = wavelength_m * 0.20
driven_to_first_director_spacing_m = wavelength_m * 0.20
director_to_director_spacing_m = wavelength_m * 0.25
```

If a boom correction is supplied, qantcal subtracts it from each element length and rejects any result that would make an element zero or negative. Boom correction is construction-specific and must be verified.

Yagi dimensions depend on element diameter, boom effects, spacing, matching method, construction material, height above ground, mounting, and surroundings. Build elements slightly long where practical, then measure and trim. The driven element feed and matching method is not designed in this pass.

Future Yagi work may add NEC or Method-of-Moments export, EZNEC/4NEC2/MMANA-style model export, element diameter correction, boom correction tables, folded dipole, gamma match, hairpin match, and model-derived gain, front-to-back, and SWR estimates.

## Reverse Length-to-Frequency Calculations

Reverse calculations invert the same formulas. For example, a dipole total length is treated as:

```text
frequency_hz = 299792458.0 / ((total_length_m / factor) * 2)
```

These results are approximate for the same reasons as the forward calculations.

## Band Table

qantcal includes a small internal ham band table for UI convenience. It stores display name, lower frequency, upper frequency, typical centre/design frequency, and broad notes.

This table is not legal operating authority. Users must check their national licence, current band plan, and local restrictions. UK-oriented users should check current RSGB and Ofcom sources.

## RF Helper Calculators

The RF helper calculators are practical starting-point tools. They are not safety, legal, or compliance guarantees.

### Air-Core Solenoid Coil

qantcal uses Wheeler's single-layer air-core coil approximation:

```text
L_uH = (N^2 * D_in^2) / (18 * D_in + 40 * l_in)
```

`D_in` is coil diameter in inches, `l_in` is coil length in inches, `N` is turns, and `L_uH` is inductance in microhenries.

This approximation does not account for wire diameter, spacing, nearby metal, ferrite cores, self-capacitance, or high-power heating.

### RF Choke

The RF choke calculator is a first-pass common-mode choke helper. It expects the user to enter measured or datasheet impedance for one pass or turn on the same core, cable, winding style, and frequency.

```text
estimated_impedance_ohms = reference_impedance_ohms * turns^2
suggested_turns = ceil(sqrt(target_impedance_ohms / reference_impedance_ohms))
```

The ferrite mix selector provides notes only. qantcal does not ship a ferrite impedance database in this pass.

This estimate is intentionally cautious. Accurate choke impedance depends on ferrite material, core geometry, cable type, turns, winding layout, frequency, self-capacitance, power, heating, and measured data. Finished chokes should be verified with suitable RF measurement equipment.

### L-Network Matching

The matching helper is a first-pass resistive L-network calculator. It assumes the source and load are purely resistive:

```text
R_high = max(source_resistance_ohms, load_resistance_ohms)
R_low = min(source_resistance_ohms, load_resistance_ohms)
Q = sqrt((R_high / R_low) - 1)
X_series = Q * R_low
X_shunt = R_high / Q
```

Component conversions use:

```text
L = X / (2 * pi * f)
C = 1 / (2 * pi * f * X)
```

qantcal shows two first-pass options:

- low-pass: series inductor on the low-resistance side and shunt capacitor across the high-resistance side
- high-pass: series capacitor on the low-resistance side and shunt inductor across the high-resistance side

This helper does not model complex feedpoint impedance, tuner loss, component Q, voltage/current ratings, stray capacitance, layout, baluns, common-mode current, or antenna interaction. Real matching networks should be designed from measured impedance at the operating frequency.

### Complex Impedance Helper

The impedance helper accepts a measured or entered complex impedance:

```text
Z = R + jX
|Z| = sqrt(R^2 + X^2)
phase_degrees = atan2(X, R) * 180 / pi
```

Admittance is calculated as:

```text
Y = 1 / Z = G + jB
G = R / (R^2 + X^2)
B = -X / (R^2 + X^2)
```

Match quality against a reference system impedance uses:

```text
Gamma = (Z - Z0) / (Z + Z0)
SWR = (1 + |Gamma|) / (1 - |Gamma|)
return_loss_db = -20 * log10(|Gamma|)
mismatch_loss_db = -10 * log10(1 - |Gamma|^2)
```

Positive reactance is shown as an equivalent series inductance:

```text
L = X / (2 * pi * f)
```

Negative reactance is shown as an equivalent series capacitance:

```text
C = 1 / (2 * pi * f * abs(X))
```

This helper analyses one impedance point only. It does not design a tuner, model component stress, import S-parameters, draw a Smith chart, or calculate a frequency sweep.

### RF Loading Coil

The RF loading coil helper reuses the same ideal resonance calculation used by LF/MF loading guidance. The user supplies antenna capacitance:

```text
L = 1 / ((2 * pi * f)^2 * C)
X_C = 1 / (2 * pi * f * C)
X_L = 2 * pi * f * L
```

Capacitance is converted from picofarads to farads, and inductance is shown in microhenries and millihenries.

This is not a full antenna model. Coil Q, self-capacitance, current and voltage rating, losses, insulation spacing, weatherproofing, antenna radiation resistance, and matching-network performance are not calculated.

### LC Resonance

The LC resonance calculator uses:

```text
f = 1 / (2 * pi * sqrt(L * C))
```

Inductance is converted from microhenries to henries, and capacitance is converted from picofarads to farads. Reverse calculations invert the same formula for inductance or capacitance when frequency and the other component value are supplied.

### LC Trap

The trap calculator is a first-pass parallel LC trap helper. It uses the same ideal LC resonance formula:

```text
f = 1 / (2 * pi * sqrt(L * C))
```

For reverse calculations:

```text
L = 1 / ((2 * pi * f)^2 * C)
C = 1 / ((2 * pi * f)^2 * L)
```

Trap component reactance at resonance is shown as:

```text
X_L = 2 * pi * f * L
X_C = 1 / (2 * pi * f * C)
```

The trap calculator does not model trap Q, ESR, coil self-capacitance, dielectric loss, voltage stress, heating, weatherproofing, enclosure effects, power handling, or interaction with antenna element lengths. Traps should be measured and adjusted with real components.

### SWR and Reflected Power

The SWR calculator uses:

```text
|Gamma| = (SWR - 1) / (SWR + 1)
reflected_power_w = forward_power_w * |Gamma|^2
reflected_percent = |Gamma|^2 * 100
delivered_power_w = forward_power_w - reflected_power_w
```

This assumes a lossless line and does not model tuner, coax, connector, or matching losses.

### Coax Loss

The coax-loss calculator expects the user to enter matched cable loss in dB per 100 m at the operating frequency. qantcal does not ship manufacturer coax tables in this pass.

Matched line loss:

```text
matched_loss_db = loss_db_per_100m * length_m / 100
matched_power_ratio = 10 ^ (-matched_loss_db / 10)
```

When a load SWR is supplied, qantcal applies a first-pass mismatch-loss estimate:

```text
reflection_coefficient = (SWR - 1) / (SWR + 1)
delivered_ratio = matched_power_ratio * (1 - reflection_coefficient^2) / (1 - reflection_coefficient^2 * matched_power_ratio^2)
total_loss_db = -10 * log10(delivered_ratio)
additional_swr_loss_db = total_loss_db - matched_loss_db
delivered_power_w = input_power_w * delivered_ratio
```

This is still a simplified estimate. Real coax loss depends on cable type, frequency, connectors, installation, age, water ingress, temperature, manufacturing tolerance, and measured cable condition.

### VHF/UHF Radio Horizon

The radio horizon calculator defaults to the common 4/3 effective Earth-radius radio-horizon model:

```text
distance_km = sqrt(17.0 * height_m)
combined_distance_km = tx_horizon_km + rx_horizon_km
```

The geometric horizon model is also available:

```text
distance_km = sqrt(12.746 * height_m)
combined_distance_km = tx_horizon_km + rx_horizon_km
```

These are line-of-sight estimates only. Real range depends on terrain, clutter, foliage, buildings, power, antenna gain, polarisation, receiver sensitivity, coax loss, mode, noise, and atmospheric conditions.

## Multi-Band Target Guidance

Multi-band target guidance is rule-based advisory text. qantcal checks the saved target list for broad cases:

- one enabled target
- multiple enabled targets
- close-spaced target frequencies
- harmonic-like target frequency ratios
- Yagi projects with multiple targets
- mixed service types

The guidance does not calculate fan dipole spacing, trap placement, trap power handling, common feedpoint impedance, element interaction, current distribution, or NEC-style model results. It only reminds the user that saved target dimensions are independent starting points and that real multi-band antennas need measurement and trimming.

## Band Reference and Propagation Notes

Band reference and propagation notes are educational context. They are not legal authority, not a propagation prediction, and not a guarantee of range or reliability.

HF reach guidance is qualitative in this pass. qantcal does not calculate fixed kilometre predictions for HF because real paths depend on frequency, time, season, solar conditions, geomagnetic conditions, path geometry, antennas, power, receiver noise, mode, and local interference.

For VHF/UHF, qantcal may show the existing radio-horizon estimate:

```text
distance_km = sqrt(17.0 * height_m)
combined_distance_km = tx_horizon_km + rx_horizon_km
```

Mode notes are qualitative only. qantcal does not implement verified SNR thresholds or exact mode performance models in this pass.

VOACAP, ITU-style engines, solar flux, and K-index workflows are future work. VOACAP online services must not be scraped or accessed automatically without prior agreement from the VOACAP site owner.

## LF/MF Antenna Guidance

LF/MF antenna guidance uses the same exact speed of light constant and reference wavelength formulas:

```text
wavelength_m = 299792458.0 / frequency_hz
quarter_wave_m = wavelength_m / 4
half_wave_m = wavelength_m / 2
full_wave_m = wavelength_m
electrical_height_ratio = physical_height_m / wavelength_m
total_wire_ratio = total_wire_length_m / wavelength_m
```

Full-size LF/MF reference dimensions are often hundreds of metres or more. qantcal shows them as references, not as practical site recommendations.

For loading-coil guidance, qantcal only calculates the ideal inductance needed to resonate a user-supplied capacitance:

```text
L_h = 1 / ((2 * pi * frequency_hz)^2 * capacitance_f)
```

This is not a full antenna model. Radiation resistance, loss resistance, ground loss, loading coil Q, antenna capacitance, feed impedance, current distribution, bandwidth, voltage stress, and matching-network performance are not solved. Top-loading length is shown as physical context only; loading inductance is calculated from the supplied capacitance. Antenna capacitance should be measured or carefully estimated before using a loading-coil value.

The LF/MF reference data includes amateur 2200m and 630m ranges, LW/MW broadcast reference ranges, and electrically short antenna concepts such as loaded verticals, inverted-L antennas, T/top-loaded verticals, and receive loops. These are starting guidance only and do not grant operating authority.

## Propagation and Reach Estimate Notes

Propagation and reach estimates are intentionally limited. Current work separates:

- simple radio horizon estimates for VHF/UHF
- simple HF band notes
- later VOACAP, ITU-style, or import/export integration
- user profile inputs such as power, mode, antenna height, location, noise level, terrain, date/time, and solar data

No first-pass UI should present reach estimates as confirmed coverage predictions.

## Legal and Safety Notes

Users must obey their licence terms, band plans, RF exposure rules, local planning restrictions, and electrical safety requirements.

For UK-oriented band-plan references, check current RSGB and Ofcom sources. qantcal should not hard-code detailed legal rules in the first scaffold.

## Sources to Check

- SI definition of the metre and speed of light in vacuum
- Online halo references describing the halo as a half-wave bent dipole with a gap opposite the feed point, including https://en.wikipedia.org/wiki/Halo_antenna and https://www.kr1st.com/2mhalo.htm
- Qt 6 documentation for Widgets, Graphics View, and Print Support
- RSGB band plans and operating guidance for UK amateur radio use
- Ofcom amateur radio licence terms and related UK rules
- VOACAP documentation for future HF propagation modelling
- ITU recommendations for future propagation model research
