#!/usr/bin/env bash
# qantcal - Developed by M6VPN (M6VPN@tuta.com)
# scripts/configure.sh

set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"

cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
