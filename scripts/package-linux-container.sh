#!/usr/bin/env bash
# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# scripts/package-linux-container.sh

set -euo pipefail

IMAGE="${1:?container image is required}"
PLATFORM="${2:?container platform is required}"
GENERATOR="${3:?package generator is required}"
LABEL="${4:?artifact label is required}"
OUTPUT_DIR="${5:?output directory is required}"
APPIMAGE_ARCH="${6:-}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

mkdir -p "${OUTPUT_DIR}"

docker run --rm \
	--platform "${PLATFORM}" \
	-e APPIMAGE_ARCH="${APPIMAGE_ARCH}" \
	-e DEBIAN_FRONTEND=noninteractive \
	-e GENERATOR="${GENERATOR}" \
	-e LABEL="${LABEL}" \
	-e OUTPUT_DIR="/workspace/${OUTPUT_DIR}" \
	-v "${REPO_ROOT}:/workspace" \
	-w /workspace \
	"${IMAGE}" \
	bash -lc '
		set -euo pipefail

		install_apt_dependencies()
		{
			apt-get update
			apt-get install -y \
				build-essential \
				ca-certificates \
				cmake \
				curl \
				dpkg-dev \
				file \
				git \
				libgl1-mesa-dev \
				ninja-build \
				qt6-base-dev \
				qt6-base-dev-tools \
				qt6-tools-dev-tools
			apt-get install -y libfuse2 || apt-get install -y libfuse2t64 || true
		}

		install_dnf_dependencies()
		{
			dnf install -y dnf-plugins-core || true
			dnf config-manager --set-enabled crb || true
			dnf install -y epel-release || true
			dnf install -y \
				ca-certificates \
				cmake \
				curl \
				file \
				gcc-c++ \
				git \
				make \
				ninja-build \
				qt6-qtbase-devel \
				qt6-qttools-devel \
				rpm-build \
				which
		}

		if command -v apt-get >/dev/null 2>&1; then
			install_apt_dependencies
		elif command -v dnf >/dev/null 2>&1; then
			install_dnf_dependencies
		else
			echo "error: unsupported package manager in container" >&2
			exit 1
		fi

		BUILD_DIR="build/release-${LABEL}"
		ARTIFACT_DIR="dist/${LABEL}"

		rm -rf "${BUILD_DIR}" "${ARTIFACT_DIR}"
		mkdir -p "${ARTIFACT_DIR}" "${OUTPUT_DIR}"

		cmake -S . -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release
		cmake --build "${BUILD_DIR}"
		QT_QPA_PLATFORM=offscreen ctest --test-dir "${BUILD_DIR}" --output-on-failure

		if [ "${GENERATOR}" = "APPIMAGE" ]; then
			./scripts/package-appimage.sh "${BUILD_DIR}" "${LABEL}" "${APPIMAGE_ARCH}" "${OUTPUT_DIR}"
		else
			cpack --config "${BUILD_DIR}/CPackConfig.cmake" -G "${GENERATOR}" -B "${ARTIFACT_DIR}"
			for artifact in "${ARTIFACT_DIR}"/*; do
				[ -f "${artifact}" ] || continue
				extension="${artifact##*.}"
				cp -v "${artifact}" "${OUTPUT_DIR}/qantcal-${LABEL}.${extension}"
			done
		fi
	'
