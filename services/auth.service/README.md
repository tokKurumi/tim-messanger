# auth-service

Authentication service for the distributed TUI messenger. Validates user credentials and issues JWT tokens for secure inter-service communication.

## Overview

-   **Responsibility**: Validate username/password (or username+token) and issue JWT tokens
-   **Auth Flow**: Receives auth requests via MQTT from `prompt.service`, verifies credentials, responds with JWT
-   **Integration**: Communicates with other microservices via MQTT for authentication events
-   **Design**: Stateless (no Redis in MVP), async, idempotent; JWT validation via shared signing keys

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

### Install Dependencies via Conan

The project uses Conan to manage external dependencies (Boost 1.89.0).

```bash
# From src/ directory

sh ./install.sh
```

This command:

-   Downloads and compiles packages (if not cached)
-   Generates files for CMake integration (`CMakeDeps`, `CMakeToolchain`)

### Build Project with CMake

After installing dependencies, build the project:

```bash
# From src/ directory

sh ./build.sh
```

### Run

```bash
./build/auth.service
```

### Docker

For containerized build and run:

```bash
docker build -t auth-service .
docker run --rm auth-service
```

Adjust environment variables for MQTT broker URL, database path, and JWT signing keys as required by your deployment.

### API / Commands (MVP)

Typical interactions exposed by `auth.service` (via MQTT):

-   `authenticate { username, password }` - Validate credentials and issue JWT token
-   `authenticate { username, token }` - Validate username+token and issue JWT
-   `validate-jwt { jwt }` - Verify JWT signature and claims (used internally by other services)

Responses include JWT with embedded user identity and claims for authorization across services.

## Notes

-   Follow repository pattern in `src/repository/` for user credential storage. Keep business logic in `src/core/` and transport/MQTT in `src/mqtt/`.
-   JWT signing keys should be loaded from secure configuration or secrets management.
-   Prefer asynchronous Boost.Asio patterns used across other services.
