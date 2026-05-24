#!/usr/bin/env bash
# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# scripts/package-windows-msys2.sh

set -euo pipefail

LABEL="${1:?artifact label is required}"
OUTPUT_DIR="${2:?output directory is required}"
BUILD_DIR="build/release-${LABEL}"
INSTALL_ROOT="package-root-${LABEL}"
STAGING_DIR="dist/${LABEL}"
ZIP_PATH="${OUTPUT_DIR}/qantcal-${LABEL}.zip"

mkdir -p "${OUTPUT_DIR}"
rm -rf "${BUILD_DIR}" "${INSTALL_ROOT}" "${STAGING_DIR}" "${ZIP_PATH}"

cmake -S . -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}"
QT_QPA_PLATFORM=offscreen ctest --test-dir "${BUILD_DIR}" --output-on-failure
cmake --install "${BUILD_DIR}" --prefix "${PWD}/${INSTALL_ROOT}"

if command -v windeployqt6 >/dev/null 2>&1; then
	windeployqt6 "${INSTALL_ROOT}/bin/qantcal.exe"
elif command -v windeployqt >/dev/null 2>&1; then
	windeployqt "${INSTALL_ROOT}/bin/qantcal.exe"
else
	echo "error: windeployqt was not found" >&2
	exit 1
fi

mkdir -p "${STAGING_DIR}/qantcal"
cp -a "${INSTALL_ROOT}/." "${STAGING_DIR}/qantcal/"

STAGING_WIN="$(cygpath -w "${STAGING_DIR}/qantcal")"
ZIP_WIN="$(cygpath -w "${ZIP_PATH}")"

powershell.exe -NoProfile -Command "Compress-Archive -Path '${STAGING_WIN}\\*' -DestinationPath '${ZIP_WIN}' -Force"
