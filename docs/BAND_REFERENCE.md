# Band Reference

qantcal includes a small band reference table to help with antenna design context. It is a convenience reference for design frequencies, broad propagation notes, and practical antenna comments.

It is not legal authority. Users must check their national regulator, licence class, current band plan, and local restrictions before transmitting. Band plans and licence terms change.

For UK use, check current RSGB and Ofcom information rather than relying on qantcal as a legal source. For international use, check your national regulator and relevant IARU regional band-plan material.

## What The Reference Includes

- amateur band name
- band service/category, such as amateur, broadcast, or informal reference
- lower and upper frequency in MHz
- typical design frequency
- broad LF/MF/HF/VHF/UHF category
- typical antenna notes
- qualitative propagation notes
- practical use-case notes

## What It Does Not Include

- legal operating permission
- licence-class permissions
- country-specific power limits
- mode sub-band enforcement
- repeater, satellite, or beacon allocations
- guaranteed propagation or range

## Shortwave Broadcast / Reference Bands

qantcal also includes a small shortwave broadcast/reference set for receive antenna design and legally authorised broadcast engineering work. These entries are not amateur allocations.

| Entry        | Range MHz   | Design MHz | Notes |
| ------------ | ----------- | ---------- | ----- |
| 75m Broadcast | 3.900-4.000 | 3.950      | Conventional shortwave broadcast reference. Night, regional, and DX listening can be possible, with high noise tendency and long antennas. |
| 49m Broadcast | 5.900-6.200 | 6.050      | Common international shortwave broadcast band. Evening, night, regional, and DX listening vary with conditions. |
| 48m Informal  | 6.200-6.500 | 6.300      | Informal/free-radio/utility listening reference around 6200-6500 kHz. It is not listed here as a standard ITU international broadcast band. |

These dimensions are suitable as starting points for receive antennas or legally authorised transmission only. qantcal does not grant authority to transmit. Band usage and legal status vary by country, service, licence, and current allocation data. Check your national regulator and current frequency allocation or band-plan material before transmitting.

## LF/MF Amateur Bands

qantcal includes LF/MF amateur reference entries for antenna starting dimensions and practical warnings.

| Entry         | Range kHz     | Design kHz | Notes |
| ------------- | ------------- | ---------- | ----- |
| 2200m Amateur | 135.7-137.8   | 136.5      | LF amateur band. Secondary allocation in many countries. Very low permitted radiated power, notification rules, and siting restrictions may apply. Antennas are usually electrically short and need high loading inductance. |
| 630m Amateur  | 472-479       | 475        | MF amateur band. Secondary allocation in many countries. Short verticals, inverted-L antennas, and T/top-loaded verticals often need loading coils and substantial ground/counterpoise systems. |

These entries are not operating permission. Check national regulator rules, licence terms, band plans, power or ERP/EIRP limits, and station permissions before transmitting.

## LW/MW Broadcast Reference Bands

LW/MW broadcast entries are for receive antennas or legally authorised broadcast engineering work. They are not amateur transmit bands.

| Entry                 | Spectrum kHz  | Carrier/Grid Note       | Design kHz |
| --------------------- | ------------- | ----------------------- | ---------- |
| LW Broadcast          | 148.5-283.5   | Region-dependent        | 198        |
| MW Broadcast Region 1/3 | 526.5-1606.5 | Often 531-1602 kHz grid | 1000       |
| MW Broadcast Americas | 525-1705      | Often 530-1700 kHz grid | 1000       |

Receive antennas can be much smaller than transmit antennas. Practical receive options include ferrite rods, tuned loops, magnetic loop receive antennas, active E-field probes, and long wires. Transmitting requires proper broadcast/legal authority and specialist design.

## Experimental / Reference LF Ranges

The 1750m LowFER / experimental reference entry covers 160-190 kHz as a legal-check-required reference range. qantcal does not call it an amateur band and does not encourage unlicensed transmission.

## Propagation Notes

Propagation notes are educational and broad. They use categories such as groundwave, NVIS, regional skywave, DX skywave, sporadic-E, tropospheric, line-of-sight, and satellite/space notes where relevant.

HF reach is qualitative in this pass. qantcal does not calculate fixed HF kilometres, path reliability, or coverage maps.

VHF/UHF guidance may include a radio-horizon estimate. That estimate is only geometric guidance and does not account for terrain, clutter, foliage, buildings, antenna gain, coax loss, receiver sensitivity, interference, or atmospheric conditions.

## Future Work

Future work may support user-supplied solar flux or Kp data, offline propagation engines, VOACAP or ITU-style local-engine workflows, terrain-aware radio-horizon mapping, path profile imports, and mode-specific link-budget calculators.
