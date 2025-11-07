# syntax=docker/dockerfile:1.5
# Stage 1: build
FROM alpine:3.22.1 AS build

RUN apk add --no-cache \
    build-base \
    linux-headers \
    make \
    cmake \
    git \
    python3 \
    py3-virtualenv \
    py3-pip \
    ccache \
    ca-certificates

# Install Conan in Python virtual environment
RUN python3 -m venv /opt/conan-venv \
    && /opt/conan-venv/bin/pip install --no-cache-dir --upgrade pip \
    && /opt/conan-venv/bin/pip install --no-cache-dir conan \
    && ln -s /opt/conan-venv/bin/conan /usr/local/bin/conan

WORKDIR /app

# Copy conanfile first for better caching
COPY conanfile.txt CMakeLists.txt ./

# Install Conan dependencies (with cached Conan home). Ensure profile exists inside cache mount.
RUN --mount=type=cache,target=/root/.conan2 \
    conan profile detect --force && \
    conan install . \
    --build=missing \
    --build=b2/* \
    -s compiler.libcxx=libstdc++11 \
    -g CMakeToolchain \
    -g CMakeDeps

# Copy source code
COPY . .

# Build with CMake using Conan toolchain
RUN --mount=type=cache,target=/root/.conan2 --mount=type=cache,target=/root/.cache/ccache \
    cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    && cmake --build build --config Release

# Stage 2: final image
FROM alpine:3.22.1

WORKDIR /app

# Support for CA certificates and necessary C++ runtime libraries
RUN apk add --no-cache ca-certificates libstdc++ libgcc && update-ca-certificates

# Copy the built binary
COPY --from=build /app/build/users.service /app/users.service

RUN chmod +x /app/users.service

CMD ["./users.service"]
