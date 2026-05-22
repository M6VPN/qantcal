# qantcal

qantcal is an early Qt6 desktop scaffold for amateur radio and RF antenna calculation. The goal is to help radio amateurs produce starting dimensions for common antennas, sketch simple designs, and later print practical build guides.

The project is in the first planning and scaffold stage. Current calculations are simple free-space formulas with a configurable shortening factor, so results are starting points only. Real antennas still need trimming, measuring, and checking in their final installation.

## Table of Contents

- [Status](#status)
- [Planned Features](#planned-features)
- [Building](#building)
- [Documentation](#documentation)
- [License](#license)

## Status

This repository currently contains the initial C++17, Qt6 Widgets, CMake, and test scaffold. It is not a complete antenna design package yet.

The first pass includes:

- a Qt Widgets main window
- basic antenna calculator inputs with a convenience ham band selector
- simple schematic diagrams using `QGraphicsScene` and `QGraphicsView`
- simple dipole, inverted Vee, vertical, EFHW, and loop calculations
- a small pure C++ calculator test executable

## Planned Features

qantcal is planned to grow into a practical RF calculator and antenna design aid with:

- antenna calculators for common wire and vertical antennas
- reverse length-to-frequency calculators
- multi-band design support
- a diagram editor for antenna layouts and printable build sheets
- printable guides and PDF exports
- RF calculators for inductors, traps, chokes, loading coils, impedance, SWR, coax loss, and link or radio horizon estimates
- propagation notes and future reach estimation

Propagation and reach estimates will start as clearly labelled rough estimates. Later design work may support pluggable engines or imports from tools such as VOACAP or ITU-style models.

## Building

See [BUILDING.md](BUILDING.md) for platform notes and CMake commands.

## Documentation

- [Project plan](docs/PROJECT_PLAN.md)
- [Formulas and sources](docs/FORMULAS_AND_SOURCES.md)

## License

qantcal is released under the ISC License. See [LICENSE](LICENSE).
