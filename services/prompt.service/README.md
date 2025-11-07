# prompt-service

SSH-based terminal UI frontend for the distributed TUI messenger. Provides the initial entry point for users to connect via SSH and interact with the messenger through a text-based interface.

## Overview

-   **SSH Server**: Accepts SSH connections and handles user authentication
-   **TUI Interface**: Provides a terminal-based user interface for messaging
-   **Commands**: Supports basic commands like `/send-message`, `/list-topics`, `/list-users`
-   **Integration**: Communicates with other microservices via MQTT for real-time messaging

## Development

### Prerequisites

-   **C++ Compiler**: GCC 11+ or Clang 12+ with C++20 support
-   **CMake**: Version 3.10 or higher
-   **Conan**: C++ package manager (version 2.x)
    -   **Installation via Python** (recommended):
        ```bash
        pip install conan
        conan profile detect --force
        ```
    -   **Installation via system package manager** (Arch Linux):
        ```bash
        sudo pacman -S conan
        conan profile detect --force
        ```
    -   **Installation via system package manager** (Ubuntu/Debian):
        ```bash
        sudo apt install python3-conan
        conan profile detect --force
        ```

### Install Dependencies & Build

The project uses Conan to manage external dependencies. Run the following command:

```bash
sh ./misc/build.sh <debug|release> ./services/prompt.service/src/
```

This command:

-   Downloads and compiles packages (if not cached)
-   Generates files for CMake integration (`CMakeDeps`, `CMakeToolchain`)
-   Creates build binary in `src/build/<Debug|Release>/prompt.service`

### Run

```bash
./services/prompt.service/src/build/<Debug|Release>/prompt.service
```

### Docker

For containerized build and run:

```bash
docker build -f ./services/prompt.service/src/Dockerfile -t prompt-service ./services/prompt.service/src/
docker run -p 2222:22 prompt-service
```

### TUI Commands (MVP)

Once connected via SSH, users can use these commands:

-   `/send-message <topic> <message>` - Send a message to a topic
-   `/change-moto <text>` - Change topic motto (if owner)
-   `/list-topics` - List available topics
-   `/list-users` - List connected users
