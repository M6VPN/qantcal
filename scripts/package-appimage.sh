#!/usr/bin/env bash
# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# scripts/package-appimage.sh

set -euo pipefail

BUILD_DIR_INPUT="${1:?build directory is required}"
LABEL="${2:?artifact label is required}"
APPIMAGE_ARCH="${3:?AppImage architecture is required}"
OUTPUT_DIR_INPUT="${4:?output directory is required}"

BUILD_DIR="$(cd "${BUILD_DIR_INPUT}" && pwd)"
APPDIR="${BUILD_DIR}/AppDir"
TOOLS_DIR="${BUILD_DIR}/appimage-tools"
TOOL_ARCH="${APPIMAGE_ARCH}"

mkdir -p "${APPDIR}" "${TOOLS_DIR}" "${OUTPUT_DIR_INPUT}"
OUTPUT_DIR="$(cd "${OUTPUT_DIR_INPUT}" && pwd)"
rm -rf "${APPDIR:?}/"*

cmake --install "${BUILD_DIR}" --prefix "${APPDIR}/usr"

download_tool()
{
	local url="${1:?url is required}"
	local output="${2:?output path is required}"

	if [ ! -f "${output}" ]; then
		curl -L --retry 3 --output "${output}" "${url}"
		chmod +x "${output}"
	fi
}

download_tool "https://github.com/linuxdeploy/linuxdeploy/releases/latest/download/linuxdeploy-${TOOL_ARCH}.AppImage" "${TOOLS_DIR}/linuxdeploy-${TOOL_ARCH}.AppImage"
download_tool "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/latest/download/linuxdeploy-plugin-qt-${TOOL_ARCH}.AppImage" "${TOOLS_DIR}/linuxdeploy-plugin-qt-${TOOL_ARCH}.AppImage"
download_tool "https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/latest/download/linuxdeploy-plugin-appimage-${TOOL_ARCH}.AppImage" "${TOOLS_DIR}/linuxdeploy-plugin-appimage-${TOOL_ARCH}.AppImage"

ln -sf "linuxdeploy-plugin-qt-${TOOL_ARCH}.AppImage" "${TOOLS_DIR}/linuxdeploy-plugin-qt"
ln -sf "linuxdeploy-plugin-appimage-${TOOL_ARCH}.AppImage" "${TOOLS_DIR}/linuxdeploy-plugin-appimage"

export APPIMAGE_EXTRACT_AND_RUN=1
export PATH="${TOOLS_DIR}:${PATH}"

(
	cd "${BUILD_DIR}"
	rm -f ./*.AppImage
	"${TOOLS_DIR}/linuxdeploy-${TOOL_ARCH}.AppImage" \
		--appdir "${APPDIR}" \
		--desktop-file "${APPDIR}/usr/share/applications/qantcal.desktop" \
		--icon-file "${APPDIR}/usr/share/icons/hicolor/scalable/apps/qantcal.svg" \
		--plugin qt \
		--output appimage
)

APPIMAGE_PATH="$(find "${BUILD_DIR}" -maxdepth 1 -type f -name '*.AppImage' | head -n 1)"
if [ -z "${APPIMAGE_PATH}" ]; then
	echo "error: AppImage was not produced" >&2
	exit 1
fi

mv "${APPIMAGE_PATH}" "${OUTPUT_DIR}/qantcal-${LABEL}.AppImage"
