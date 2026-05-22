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
- [Tests](#tests)

## Ubuntu and Debian

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools
```

## Fedora

```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtbase-devel-tools
```

## Arch Linux

```bash
sudo pacman -S --needed base-devel cmake qt6-base
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

## Tests

```bash
ctest --test-dir build --output-on-failure
```
