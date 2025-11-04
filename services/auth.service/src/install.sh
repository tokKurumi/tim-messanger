#!/usr/bin/env bash
set -euo pipefail

# Always run from this script's directory (should contain conanfile.txt)
cd "$(dirname "${BASH_SOURCE[0]}")"

# Build type can be passed as first arg (Release by default)
BUILD_TYPE="${1:-Release}"

# Detect Conan profile (idempotent)
conan profile detect --force >/dev/null 2>&1 || true

echo "[install.sh] Installing Conan dependencies (build_type=${BUILD_TYPE})..."

conan install . \
    --build=missing \
    -s compiler.libcxx=libstdc++11 \
    -s build_type="${BUILD_TYPE}" \
    -g CMakeToolchain \
    -g CMakeDeps

echo "[install.sh] Done. Toolchain: build/${BUILD_TYPE}/generators/conan_toolchain.cmake"
