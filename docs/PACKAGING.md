# Packaging qantcal

qantcal does not have installers yet.

## CMake Install

After building, install to a staging prefix:

```bash
cmake --install build --prefix "$PWD/package-root"
```

This installs the `qantcal` executable and project documentation. Runtime Qt libraries are still supplied by the operating system or the Qt deployment tools for the target platform.

## Linux

For local package staging, use the CMake install command above.

Future Linux packaging:

- AppImage
- Flatpak
- distro packages

## Windows

Windows deployment should use Qt deployment tools such as `windeployqt` after a successful build. A typical workflow is:

1. Build qantcal with the same Qt kit that will be deployed.
2. Copy or install the executable into a staging directory.
3. Run `windeployqt` against the staged executable.
4. Test the staged directory on a clean machine or VM.

No Windows installer is implemented yet.

## FreeBSD and OpenBSD

For now, build from source using packages or ports for CMake, a C++17 compiler, and Qt6 base libraries. Packaging for ports is future work.

## Release Automation

qantcal does not publish release artifacts from CI yet.
