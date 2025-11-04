# auth-service

Authentication service for the distributed TUI messenger. Handles SSH keys, generates JWT tokens, and interacts with Redis for session storage.

## Development

### Prerequisites
- **C++ Compiler**: GCC 11+ or Clang 12+ with C++20 support
- **CMake**: Version 3.10 or higher
- **Conan**: C++ package manager (version 2.x)
  - **Installation via Python** (recommended):
    ```bash
    pip install conan
    conan profile detect --force
    ```

### Install Dependencies via Conan
The project uses Conan to manage external dependencies (Boost 1.89.0).

```bash
# From src/ directory

conan install . \
    --build=missing \
    -s compiler.libcxx=libstdc++11 \
    -g CMakeToolchain \
    -g CMakeDeps
```

This command:
- Downloads and compiles packages (if not cached)
- Generates files for CMake integration (`CMakeDeps`, `CMakeToolchain`)

### Build Project with CMake
After installing dependencies, build the project:

```bash
# From src/ directory

cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc" \
    && cmake --build build --config Release
```

### Run
```bash
./build/auth.service
```

### Docker
For containerized build:
```bash
docker build -t auth-service .
docker run --rm auth-service
```