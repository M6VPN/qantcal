#!/usr/bin/env bash
# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# scripts/build.sh

set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"

cmake --build "${BUILD_DIR}"
