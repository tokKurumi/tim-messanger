# Stage 1: build
FROM debian:bookworm-slim AS build

RUN apt update && apt install -y --no-install-recommends \
    build-essential \
    make \
    cmake \
    git \
    python3 \
    python3-venv \
    python3-pip \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install Conan in Python virtual environment
RUN python3 -m venv /opt/conan-venv \
    && /opt/conan-venv/bin/pip install --no-cache-dir --upgrade pip \
    && /opt/conan-venv/bin/pip install --no-cache-dir conan \
    && ln -s /opt/conan-venv/bin/conan /usr/local/bin/conan

# Create Conan default profile
RUN conan profile detect --force

WORKDIR /app

# Copy conanfile first for better caching
COPY conanfile.txt CMakeLists.txt ./

# Install Conan dependencies
RUN conan install . \
    --build=missing \
    -s compiler.libcxx=libstdc++11 \
    -g CMakeToolchain \
    -g CMakeDeps

# Copy source code
COPY . .

# Build with CMake using Conan toolchain
RUN cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc -static" \
    && cmake --build build --config Release

# Stage 2: final image
FROM alpine:3.22.1

WORKDIR /app

# Support for CA certificates
RUN apk add --no-cache ca-certificates && update-ca-certificates

COPY --from=build /app/build/prompt.service /app/prompt.service

RUN chmod +x /app/prompt.service

EXPOSE 22

CMD ["./prompt.service"]
