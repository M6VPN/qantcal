#!/usr/bin/env bash
# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# scripts/test.sh

set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}"

ctest --test-dir "${BUILD_DIR}" --output-on-failure
