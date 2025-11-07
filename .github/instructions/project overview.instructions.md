---
applyTo: '**'
---

# Project context

This repository implements a distributed, server-side TUI messenger accessible via SSH.
The system consists of stateless C++20 microservices communicating asynchronously via MQTT.
The server-side TUI frontend communicates with broker via MQTT.
Authentication is handled by a dedicated auth-service using username/password (or username+token), and JWT is used for identity and claims propagation between services.
All services are containerized and deployable via Docker Swarm.

# Language and technology stack

-   Language: C++20
-   Build system: CMake
-   Package manager: Conan 2.x
-   Concurrency: asynchronous, event-driven, multi-threaded
-   Communication:
    -   Internal: MQTT (async, pub/sub)
    -   External: SSH (client → TUI frontend)
-   Database: SQLite (local per service)
-   Logging: spdlog
-   Allowed libraries:
    -   Boost (Asio, Beast, Thread, Filesystem)
    -   SQLite3
    -   libmosquitto
    -   libssh (server-side SSH)
    -   ftxui / ncurses (TUI)
-   Minimal dependencies
-   Pimpl idiom for all public interfaces
-   Repository pattern for data access

# Package management with Conan

-   **Conan 2.x** is used for all external C++ dependencies
-   Each microservice has its own `conanfile.txt` or `conanfile.py`
-   All external libraries (Boost, SQLite3, spdlog, etc.) are managed via Conan
-   No system-wide library installations required
-   Conan integration with CMake via `CMakeDeps` and `CMakeToolchain` generators

## Conan workflow

1. Define dependencies in `conanfile.txt`:

```ini
   [requires]
   boost/1.89.0

   [generators]
   CMakeDeps
   CMakeToolchain

   [layout]
   cmake_layout
```

2. Install dependencies:

```bash
   conan install . --build=missing -s compiler.libcxx=libstdc++11
```

3. Build with CMake using Conan toolchain:

```bash
   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
```

## Docker integration

-   Conan is installed in a Python virtual environment during Docker build
-   Dependencies are installed during Docker build
-   Conan cache can be mounted as a volume to speed up builds
-   Example Dockerfile pattern:

```dockerfile
  FROM debian:bookworm-slim AS build
  RUN apt update && apt install -y --no-install-recommends \
      build-essential make cmake git python3 python3-venv python3-pip ca-certificates \
      && rm -rf /var/lib/apt/lists/*
  RUN python3 -m venv /opt/conan-venv \
      && /opt/conan-venv/bin/pip install --no-cache-dir --upgrade pip \
      && /opt/conan-venv/bin/pip install --no-cache-dir conan \
      && ln -s /opt/conan-venv/bin/conan /usr/local/bin/conan
  RUN conan profile detect --force
  WORKDIR /app
  COPY conanfile.txt CMakeLists.txt ./
  RUN conan install . --build=missing -s compiler.libcxx=libstdc++11 -g CMakeToolchain -g CMakeDeps
  COPY . .
  RUN cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release \
      && cmake --build build --config Release
```

## Conan profiles

-   Use Conan profiles for different build configurations
-   Default profile for Linux/GCC builds
-   Custom profiles for cross-compilation if needed
-   Profile example:

```ini
  [settings]
  os=Linux
  arch=x86_64
  compiler=gcc
  compiler.version=15
  compiler.libcxx=libstdc++11
  build_type=Release
```

# Distributed system principles

-   All microservices (except the MQTT broker) are stateless
-   Idempotent operations required:
    -   Repeated requests do not change system state beyond first application
    -   Retry-safe network calls, DB operations, message delivery
-   System must tolerate multiple points of failure:
    -   Service crash or restart
    -   Network partition
    -   MQTT broker outage
    -   Frontend disconnection
-   Asynchronous retry and backoff strategies
-   Stateless services allow replication and failover

# Microservices overview

## 1. prompt.service (entry point)

-   SSH-based entrypoint for end-users (default: port 2222)
-   Accepts SSH connections like: `ssh -p 2222 <username>@<host>`; collects username and password (or token)
-   Sends credentials to auth-service via MQTT for verification
-   On success: greets user (e.g., `hello <user>!`) and provides minimal TUI commands
-   Stateless by design; does not use system key storage (ephemeral host key for MVP, later persistent shared key across multiple instances)
-   Concurrency: Boost.Asio powered worker pool; accepts multiple SSH sessions concurrently
-   Idempotent command handling

## 2. auth.service

-   Validates username/password (or username+token) requests coming from prompt.service via MQTT
-   Issues JWT on success; JWT is used for calling other microservices
-   Stateless (no Redis); JWT validation is done via shared signing keys
-   Idempotent authentication flow

## 3. user.service

-   Manages user profile data (non-sensitive)
-   Requires valid JWT
-   Stateless, async, multi-threaded
-   Idempotent updates

## 4. topic.service

-   Business entity: Topic
    -   Each user can create a topic
    -   Topic has a name, slogan (moto), participants
    -   Only owner can change moto or delete the topic
    -   Other users can post messages in the topic
-   Publishes/subscribes via MQTT
-   Stateless, async
-   Idempotent message handling
-   MQTT message payload includes JWT, request data, idempotency key

## 5. migrator.service (universal)

-   Standalone CLI for schema migrations, reused per microservice
-   Supports bidirectional migrations: `up`, `down`, `to <version>`, `status`
-   Conventions:
    -   A folder with SQL migrations scripts
    -   Each version has two scripts: `<version>_up.sql` and `<version>_down.sql`
    -   A config maps versions and ordering
-   Fully asynchronous; idempotent operations
-   For each microservice use its own instance of migrator with its own DB (connection) and migration scripts, so that migrations are isolated.

# Internal communication (microservice-to-microservice)

-   **MQTT**: async pub/sub for commands and events
-   MQTT messages include:
    -   JWT
    -   Request data
    -   Idempotency key

# External communication (client → system)

-   **SSH**: only channel for connecting to the system via prompt.service (default: port 2222)
-   Login flow: username/password (or token) captured in SSH session → verified by auth.service via MQTT → on success, TUI greets the user
-   TUI commands (MVP): `/send-message`, `/change-moto`, `/list-topics`, `/list-users`

# Fault-tolerance and resilience

-   Services detect and recover from transient failures
-   MQTT broker failures: retry connections, idempotent message handling
-   SSH client disconnections: clients can reconnect; JWT is stateless and can be revalidated
-   Database writes are transactional

# Deployment

-   Each service containerized separately
-   Docker Swarm stack:
    -   prompt.service + migrator
    -   auth.service + migrator
    -   user.service + migrator
    -   topic.service + migrator
    -   MQTT broker
-   Volumes for SQLite persistence
-   Secrets for JWT keys

# C++ design rules

-   Layers: API, core logic, repository, MQTT
-   Async via Boost.Asio (coroutines or handler-based)
-   HTTP via Boost.Beast (if needed, prefer MQTT for inter-service communication)
-   Thread pools configurable
-   Pimpl pattern for all public classes
-   Error handling mandatory; all operations idempotent
-   Graceful shutdown via signals
-   Non-blocking logging via spdlog

## Code organization and structure

-   All source code for each microservice must reside strictly under its `src/` directory.
-   Keep code well-structured and class-oriented; favor small, cohesive classes with clear responsibilities (SRP).
-   Reuse internal services/components where practical to avoid duplication; prefer composition over inheritance.
-   Apply the repository pattern for all database access; business logic and transport layers must never talk to the DB directly.
-   Maintain clean internal boundaries between layers (API ↔ core ↔ repository ↔ transport/MQTT/SSH).
-   Public interfaces use Pimpl; internal headers stay inside `src/` with minimal exposure.
-   Prefer consistent module folders under `src/` (suggested):
    -   `src/api/` – DTOs, request/response contracts
    -   `src/core/` – domain services, use-cases, business rules
    -   `src/repository/` – repositories, mappers, SQL
    -   `src/mqtt/` – MQTT client, topics, serializers
    -   `src/ssh/` – SSH server(frontend) pieces where applicable
    -   `src/utils/` – small shared helpers (keep minimal)
-   Keep functions small, pure where possible, and testable; avoid side effects leaking across layers.

# Non-goals

-   No Telnet or plaintext protocols
-   No monolithic or synchronous services
-   No global shared state outside MQTT
