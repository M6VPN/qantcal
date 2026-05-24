# Building qantcal

qantcal uses C++17, CMake, and Qt6 Widgets. Exact Qt package names vary by distribution and version, so check your package manager if one of these names has changed.

## Table of Contents

- [Ubuntu and Debian](#ubuntu-and-debian)
- [Fedora](#fedora)
- [Arch Linux](#arch-linux)
- [OpenBSD](#openbsd)
- [FreeBSD](#freebsd)
- [Windows](#windows)
- [Generic CMake Build](#generic-cmake-build)
- [CMake Presets](#cmake-presets)
- [Helper Scripts](#helper-scripts)
- [Tests](#tests)
- [Install](#install)

## Ubuntu and Debian

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools qt6-tools-dev-tools
```

## Fedora

```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qttools-devel
```

## Arch Linux

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-tools
```

## OpenBSD

```sh
doas pkg_add cmake qt6-qtbase
```

OpenBSD package names can differ by release. Use `pkg_info -Q qt6` if the package name has changed.

## FreeBSD

```sh
sudo pkg install cmake qt6-base
```

## Windows

The easiest first build path on Windows is Qt Creator:

1. Install Qt 6 from the Qt online installer.
2. Include a C++ compiler kit such as MSVC or MinGW.
3. Open this repository as a CMake project in Qt Creator.
4. Configure, build, and run the `qantcal` target.

Command-line CMake is also possible from a terminal where Qt and the compiler are already on `PATH`:

```bat
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
build\src\Release\qantcal.exe
```

The output path can vary by generator. Visual Studio generators usually place the executable under a configuration folder such as `Release`.

## Generic CMake Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/src/qantcal
```

## CMake Presets

The repository includes CMake presets for common local and CI builds:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The `release` and `ci-debug` presets are also available. Presets do not hard-code local Qt paths.

Manual CMake commands remain supported for older CMake versions. Presets require a CMake version with preset support.

## Helper Scripts

Small local wrappers are provided for the default Debug build:

```bash
./scripts/configure.sh
./scripts/build.sh
./scripts/test.sh
```

The scripts accept `BUILD_DIR` and `BUILD_TYPE` environment variables:

```bash
BUILD_DIR=build-release BUILD_TYPE=Release ./scripts/configure.sh
BUILD_DIR=build-release ./scripts/build.sh
BUILD_DIR=build-release ./scripts/test.sh
```

The scripts do not install packages and do not use `sudo`.

## Tests

```bash
ctest --test-dir build --output-on-failure
```

For headless systems or CI, use Qt's offscreen platform:

```bash
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

## Install

Install to a local prefix or staging directory:

```bash
cmake --install build --prefix "$PWD/package-root"
```

See [Packaging qantcal](docs/PACKAGING.md) for current deployment notes.
