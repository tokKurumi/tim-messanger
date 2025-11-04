#!/usr/bin/env bash
set -euo pipefail

# Always run from this script's directory (should contain CMakeLists.txt)
cd "$(dirname "${BASH_SOURCE[0]}")"

# Build type can be passed as first arg (Release by default)
BUILD_TYPE="${1:-Release}"

TOOLCHAIN_PATH="build/${BUILD_TYPE}/generators/conan_toolchain.cmake"

if [[ ! -f "${TOOLCHAIN_PATH}" ]]; then
    echo "[build.sh] Conan toolchain not found: ${TOOLCHAIN_PATH}" >&2
    echo "[build.sh] Run ./install.sh ${BUILD_TYPE} first to generate it." >&2
    exit 1
fi

echo "[build.sh] Configuring (build_type=${BUILD_TYPE})..."

cmake -S . -B build \
        -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_PATH}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc"

echo "[build.sh] Building..."

cmake --build build --config "${BUILD_TYPE}"

echo "[build.sh] Done. Binary should be at build/prompt.service"
