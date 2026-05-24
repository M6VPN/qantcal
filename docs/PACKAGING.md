# Packaging qantcal

qantcal packaging is built through CMake install rules, CPack, and the GitHub release workflow.

## First Alpha Release Gate

Before tagging the first alpha:

- build with the `release` preset
- run the full test suite with Qt offscreen enabled
- run `cmake --install` into a clean staging prefix
- confirm the staged `qantcal` executable starts on the target platform
- confirm `docs/`, `BUILDING.md`, `README.md`, and `LICENSE` are installed
- confirm translation catalogs are installed when `lrelease` is available
- export at least one antenna guide PDF and inspect the diagram
- verify the GitHub release workflow creates a draft prerelease
- note that packages are unsigned for the first alpha

## CMake Install

After building, install to a staging prefix:

```bash
cmake --install build --prefix "$PWD/package-root"
```

This installs the `qantcal` executable, project documentation, Linux desktop metadata, icon metadata, and translation catalogs. Runtime Qt libraries are supplied by the operating system for DEB/RPM packages and by deployment tools for AppImage and Windows ZIP artifacts.

## Linux

For local package staging, use the CMake install command above.

The release workflow builds:

- AppImage for x86_64, aarch64, and armv7l
- DEB packages for Ubuntu 24.04 and Debian trixie on amd64, arm64, and armhf
- RPM packages for Fedora latest and Rocky latest on x86_64 and aarch64

Armbian users should use the Debian or Ubuntu package matching their base distribution and architecture.

## Windows

Windows deployment uses Qt deployment tools such as `windeployqt` after a successful build. A typical workflow is:

1. Build qantcal with the same Qt kit that will be deployed.
2. Copy or install the executable into a staging directory.
3. Run `windeployqt` against the staged executable.
4. Compress the staged directory into a portable ZIP.
5. Test the staged directory on a clean machine or VM.

The release workflow builds portable ZIP archives for x64 and arm64. No Windows installer is implemented yet.

## FreeBSD and OpenBSD

For now, build from source using packages or ports for CMake, a C++17 compiler, and Qt6 base libraries. Packaging for ports is future work.

## Release Automation

CI verifies build, tests, and CMake install staging on Linux and Windows.

The release workflow runs on release tags and manual dispatch. It builds all alpha artifacts, uploads them as workflow artifacts, and creates or updates a draft prerelease. The draft should be inspected before manual publication.
