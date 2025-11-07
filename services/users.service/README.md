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

### Install Dependencies & Build

The project uses Conan to manage external dependencies. Run the following command:

```bash
sh ./misc/build_service.sh <debug|release> ./services/users.service/src/
```

This command:

-   Downloads and compiles packages (if not cached)
-   Generates files for CMake integration (`CMakeDeps`, `CMakeToolchain`)
-   Creates build binary in `src/build/<Debug|Release>/users.service`

### Run

```bash
./services/users.service/src/build/<Debug|Release>/users.service
```

### Docker

Containerized build and run example:

```bash
docker build -f ./services/users.service/src/Dockerfile -t users-service ./services/users.service/src/
docker run users-service
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
