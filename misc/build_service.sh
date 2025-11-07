#!/usr/bin/env bash
set -euo pipefail

# build.sh
# Unified installer+builder for CMake+Conan C++ services in this repo.
# 
# This script automates the complete build workflow:
# 1. Validates input arguments and required files (conanfile.txt, CMakeLists.txt)
# 2. Installs Conan dependencies using cmake_layout into build/Debug or build/Release
# 3. Configures CMake with Conan-generated toolchain
# 4. Builds the project binary

print_usage() {
  cat <<'EOF'
Usage:
  build.sh <debug|release> <source_dir>

Positional args:
  <debug|release>   Build type. Only these two are supported. Case-insensitive.
  <source_dir>      Path to directory that contains CMakeLists.txt and conanfile.txt

Examples:
  sh misc/build.sh debug services/auth.service/src
  sh misc/build.sh release ./services/prompt.service/src
EOF
}

err() { echo "[build] ERROR: $*" >&2; }
info() { echo "[build] $*"; }

if [[ ${#} -lt 2 ]]; then
  err "Not enough arguments."
  print_usage
  exit 2
fi

# Normalize build type (case-insensitive input, capitalized output for Conan layout)
BUILD_TYPE_RAW="$1"; shift
SRC_DIR_INPUT="$1"; shift || true

BUILD_TYPE_LC="${BUILD_TYPE_RAW,,}"
case "${BUILD_TYPE_LC}" in
  debug)   CMAKE_BUILD_TYPE="Debug" ;;
  release) CMAKE_BUILD_TYPE="Release" ;;
  *)
    err "Unsupported build type: '${BUILD_TYPE_RAW}'. Only 'debug' or 'release' are allowed."
    print_usage
    exit 3
    ;;
esac

# Resolve absolute source path (without following symlinks aggressively)
if [[ ! -d "${SRC_DIR_INPUT}" ]]; then
  err "Source directory does not exist: ${SRC_DIR_INPUT}"
  exit 4
fi
SRC_DIR="$(cd "${SRC_DIR_INPUT}" && pwd)"

# Validate required files
CONANFILE_TXT="${SRC_DIR}/conanfile.txt"
CMAKELISTS_TXT="${SRC_DIR}/CMakeLists.txt"
if [[ ! -f "${CONANFILE_TXT}" ]]; then
  err "Missing conanfile.txt in ${SRC_DIR}"
  exit 5
fi
if [[ ! -f "${CMAKELISTS_TXT}" ]]; then
  err "Missing CMakeLists.txt in ${SRC_DIR}"
  exit 6
fi

# Build directory structure: build/Debug or build/Release (capitalized to match Conan conventions)
BUILD_DIR="${SRC_DIR}/build/${CMAKE_BUILD_TYPE}"
mkdir -p "${BUILD_DIR}"

# Detect Conan profile (idempotent)
conan profile detect --force >/dev/null 2>&1 || true

# Install Conan dependencies
info "Installing Conan dependencies (type=${CMAKE_BUILD_TYPE})..."
# Run conan install with cmake_layout from conanfile.txt.
# The cmake_layout directive automatically creates: build/${CMAKE_BUILD_TYPE}/generators/
# We pass the source directory explicitly to avoid changing working directory.
conan install "${SRC_DIR}" \
  --build=missing \
  -s compiler.libcxx=libstdc++11 \
  -s build_type="${CMAKE_BUILD_TYPE}" \
  -g CMakeToolchain \
  -g CMakeDeps

# Verify that Conan generated the toolchain file in expected location
# cmake_layout places generators at: ${SRC_DIR}/build/${CMAKE_BUILD_TYPE}/generators/
TOOLCHAIN_PATH="${BUILD_DIR}/generators/conan_toolchain.cmake"
if [[ ! -f "${TOOLCHAIN_PATH}" ]]; then
  err "Conan toolchain not found at expected path: ${TOOLCHAIN_PATH}"
  exit 7
fi

# Configure CMake project using Conan-generated toolchain
# The toolchain provides all dependency paths, compiler flags, and build settings
info "Configuring CMake (type=${CMAKE_BUILD_TYPE})..."
cmake -S "${SRC_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_PATH}" \
  -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"

# Build the project (compiles sources and links binary)
info "Building..."
cmake --build "${BUILD_DIR}" --config "${CMAKE_BUILD_TYPE}"

info "Done. Artifacts are in: ${BUILD_DIR}"
