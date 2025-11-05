# users-service

Service that manages user profiles for the distributed TUI messenger. Provides CRUD operations for non-sensitive user data and exposes a simple API for other microservices.

## Overview

-   **Responsibility**: Manage user profile data (create, read, update, list)
-   **Auth**: Requires valid JWT issued by `auth.service` for requests
-   **Integration**: Communicates with other microservices via MQTT for events and RPC-style requests
-   **Design**: Stateless, asynchronous, idempotent operations; repository pattern for DB access

## Development

### Prerequisites

-   **C++ Compiler**: GCC 11+ or Clang 12+ with C++20 support
-   **CMake**: Version 3.10 or higher
-   **Conan**: C++ package manager (version 2.x)
    -   **Install via Python** (recommended):
        ```bash
        pip install conan
        conan profile detect --force
        ```
    -   **Install via system package manager** (Ubuntu/Debian):
        ```bash
        sudo apt install python3-conan
        conan profile detect --force
        ```

### Install Dependencies via Conan

The project uses Conan to manage external dependencies. From the `src/` directory run:

```bash
# From src/ directory

conan install . \
    --build=missing \
    -s compiler.libcxx=libstdc++11 \
    -g CMakeToolchain \
    -g CMakeDeps
```

This command:

-   Downloads and compiles packages (if not cached)
-   Generates files for CMake integration (`CMakeDeps`, `CMakeToolchain`)

### Build Project with CMake

After installing dependencies, build the project:

```bash
# From src/ directory

cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release
```

This will create the `users.service` binary under `src/build/` (or similar build output configured in CMake).

### Run

```bash
./build/users.service
```

The service will start and connect to the MQTT broker as configured in its configuration files. It expects JWT-based requests from other services (for example, `prompt.service` and `topic.service`).

### Docker

Containerized build and run example:

```bash
docker build -t users-service .
docker run -p 0:0 users-service
```

Adjust the exposed ports and environment variables for MQTT broker URL, database path, and JWT signing keys as required by your deployment.

### API / Commands (MVP)

Typical interactions exposed by `users.service` (via MQTT RPC or internal API):

-   `create-user { username, display_name, bio }` - Create a new user profile
-   `get-user { username | user_id }` - Retrieve profile information
-   `update-user { user_id, fields... }` - Update profile fields (idempotent)
-   `list-users` - List users or search by criteria

Requests must include a valid JWT; responses follow the project's DTO conventions.

## Notes

-   Follow repository pattern in `src/repository/` for any DB access. Keep business logic in `src/core/` and transport/MQTT in `src/mqtt/`.
-   Prefer asynchronous Boost.Asio patterns used across other services.

```

```
