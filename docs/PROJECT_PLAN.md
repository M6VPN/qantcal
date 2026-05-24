# qantcal Project Plan

qantcal is planned as a cross-platform Qt6 desktop application for amateur radio and RF antenna calculation, antenna design visualisation, and printable design guides.

## Scope for the Initial Scaffold

- Qt6 Widgets application shell
- CMake project using C++17
- modular source layout
- pure C++ calculator logic separate from the UI
- antenna design scene using Qt Graphics View
- menu actions for print and PDF export
- simple tests for first-pass antenna formulas
- JSON project files for saving multi-band design state with advisory fan dipole, trap, and common feedpoint guidance
- editable design canvas with movable items, zoom, and movement undo/redo
- printable guide rendering with structured sections, page breaks, diagram rendering, and PDF export
- amateur and broadcast/reference band reference with cautious propagation notes panels
- LF/MF antenna guidance for physically-short antennas, loading coils, and receive-only compact antennas
- atmospheric map tab for remote Hepburn tropo forecast images with daily local caching
- Qt Linguist translation support, starting with Russian translation source files

## Next Improvement Priorities

Active priority: RF correctness and construction drawing hardening.

This pass should keep the application trustworthy before adding broader antenna modelling. The main goals are to clarify span versus conductor length, improve loop construction dimensions, make radio-horizon assumptions explicit, and ensure saved and exported drawings match the selected antenna type.

## First Alpha Readiness

The first alpha should be tagged only after these checks pass:

- full local and CI test suites pass on Linux and Windows
- CMake install staging succeeds on Linux and Windows
- the application reports the alpha version in Qt application metadata and the About dialog
- generated construction-guide PDFs include readable antenna drawings and labels
- README, BUILDING, PACKAGING, project-plan, formula/source, and band-reference docs are present in the release source tree
- remote tropo map fetching uses a clear qantcal User-Agent and daily cache behaviour

Items that can remain after the first alpha:

- native installers and published CI release artifacts
- wider runtime translation coverage for calculator result text
- NEC or Method-of-Moments modelling
- richer dimensioned construction drawings
- formal platform packages for Linux, Windows, FreeBSD, and OpenBSD

## Internationalisation

qantcal uses Qt Linguist `.ts` translation source files. Russian is the first starter translation.

Translation contributors should commit `translations/qantcal_<locale>.ts` files. The CMake build compiles those source files into `.qm` runtime catalogs when `lrelease` is available. Generated `.qm` files are build artifacts and should not be committed.

The app defaults to the user's environment language and falls back to English when no matching catalog is available. Users can switch between system default, English, and Russian at runtime from the Language menu.

The initial translation pass covers the application shell, menus, and tab labels. Calculator result text, warnings, guide text, and reference data still need wider translation coverage.

## Application Areas

### Antenna Calculators

Initial calculators cover:

- half-wave dipole
- folded dipole
- halo
- quarter-wave vertical
- end-fed half-wave
- full-wave loop
- random-wire advisory guidance without resonant cut-length claims
- Yagi starting-dimension designer for 2 to 10 elements

Future calculators may add off-centre-fed dipoles, fan dipoles, traps, loaded antennas, and matching network helpers.

The first Yagi tool should stay an empirical starting-dimension calculator. NEC modelling, gain optimisation, matching design, and radiation plots are future work.

Multi-band project targets are saved as independent calculated elements in the first project format. qantcal gives advisory fan dipole, trap, common feedpoint, and target-spacing guidance, but fan dipole spacing, trap placement, common feedpoint impedance, element interaction, and NEC-style modelling are future work.

### Reverse Calculators

The scaffold supports frequency-to-length and length-to-frequency modes. Reverse calculations are useful when a user has a fixed wire length and wants to estimate the nearest starting frequency.

### Diagram Editor

The design area uses `QGraphicsScene` and `QGraphicsView` because Qt Graphics View supports custom 2D items, interaction, zooming, rotation, multiple views, and scene rendering.

The design area supports generated schematic diagrams and movable saved items. Folded dipoles, loops, verticals, random wires, inverted V antennas, EFHW antennas, and Yagi elements have distinct starting-layout descriptors for project diagrams. Drawing work should preserve feedpoint placement from saved descriptors and keep PDF diagrams aligned with the saved construction layout. Later work can add richer explicit dimensions, feed points, insulators, masts, radials, and layout-specific item types.

### Printable Guides

Qt Print Support is included for cross-platform printing and PDF generation.

Printable guide support includes:

- design dimensions
- material lists
- construction notes
- trim and test notes
- safety and legal reminders
- exported PDF build sheets
- diagram snapshots rendered from saved project drawing descriptors

### RF Calculators

Current RF calculators include:

- inductors
- traps
- chokes
- matching helpers
- complex impedance helpers
- loading coils
- SWR helpers
- coax loss
- link estimates

No additional RF calculators are listed for the current plan.

### Propagation and Reach Estimates

The initial application must not present propagation guesses as facts. First-pass reach features should be labelled as rough estimates.

Planned architecture:

- simple radio horizon estimate for VHF/UHF using a clearly labelled model
- simple HF band notes
- remote Hepburn tropo forecast map viewing with selectable regions and forecast offsets
- later VOACAP, ITU-style, or import/export integration
- user profile inputs for power, mode, antenna height, location, noise level, terrain, date/time, and solar data

The first reference panel should stay qualitative for HF and use only simple radio-horizon guidance for VHF/UHF. Remote map support should fetch direct forecast images respectfully, cache by local date, and avoid scraping or automating interactive services.

Shortwave broadcast/reference support is for receive antenna design and legally authorised transmission only. Broadcast entries must not be presented as amateur allocations. The informal 48m entry is a listening/reference range around 6200-6500 kHz, not a standard ITU international broadcast band entry.

LF/MF support should remain advisory. It may calculate reference dimensions, electrical height ratios, and capacitance-based loading inductance, but it must not claim antenna efficiency, ERP/EIRP, field strength, radiation resistance, bandwidth, Q, or matching-network performance.

## Safety and Legal Notes

Users must obey their licence terms, band plans, RF exposure rules, local planning restrictions, and electrical safety requirements.

For UK-oriented use, users should check current RSGB and Ofcom material rather than relying on hard-coded legal rules in qantcal.

## Development Rules

- keep UI and calculation logic separate
- keep formulas documented
- prefer small, testable modules
- avoid external RF libraries until the calculator surface is better defined
- label empirical approximations clearly
