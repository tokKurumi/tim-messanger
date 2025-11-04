# prompt-service

SSH-based terminal UI frontend for the distributed TUI messenger. Provides the initial entry point for users to connect via SSH and interact with the messenger through a text-based interface.

## Overview
- **SSH Server**: Accepts SSH connections and handles user authentication
- **TUI Interface**: Provides a terminal-based user interface for messaging
- **Commands**: Supports basic commands like `/send-message`, `/list-topics`, `/list-users`
- **Integration**: Communicates with other microservices via MQTT for real-time messaging

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
  - **Installation via system package manager** (Arch Linux):
    ```bash
    sudo pacman -S conan
    conan profile detect --force
    ```
  - **Installation via system package manager** (Ubuntu/Debian):
    ```bash
    sudo apt install python3-conan
    conan profile detect --force
    ```

### Install Dependencies via Conan
The project uses Conan to manage external dependencies

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
./build/prompt.service
```

The service will start an SSH server (default port 22) and accept connections.

### Docker
For containerized build and run:
```bash
docker build -t prompt-service .
docker run -p 2222:22 prompt-service
```

### TUI Commands (MVP)
Once connected via SSH, users can use these commands:
- `/send-message <topic> <message>` - Send a message to a topic
- `/change-moto <text>` - Change topic motto (if owner)
- `/list-topics` - List available topics
- `/list-users` - List connected users