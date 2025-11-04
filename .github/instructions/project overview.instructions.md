---
applyTo: '**'
---
# Project context
This repository implements a distributed, server-side TUI messenger accessible via SSH.
The system consists of stateless C++20 microservices communicating asynchronously via MQTT.
The server-side TUI frontend communicates with broker via MQTT.
Authentication is based on SSH keys, and JWT is used for identity and claims propagation.
All services are containerized and deployable via Docker Swarm.

# Language and technology stack
- Language: C++20
- Build system: CMake
- Package manager: Conan 2.x
- Concurrency: asynchronous, event-driven, multi-threaded
- Communication:
  - Internal: MQTT (async, pub/sub)
  - External: SSH (client → TUI frontend)
- Database: SQLite (local per service)
- Session store: Redis (JWT, TTL 15 minutes)
- Logging: spdlog
- Allowed libraries:
  - Boost (Asio, Beast, Thread, Filesystem)
  - SQLite3
  - libmosquitto
  - ftxui / ncurses (TUI)
- Minimal dependencies
- Pimpl idiom for all public interfaces
- Repository pattern for data access

# Package management with Conan
- **Conan 2.x** is used for all external C++ dependencies
- Each microservice has its own `conanfile.txt` or `conanfile.py`
- All external libraries (Boost, SQLite3, spdlog, etc.) are managed via Conan
- No system-wide library installations required
- Conan integration with CMake via `cmake_find_package` or `CMakeDeps` generator

## Conan workflow
1. Define dependencies in `conanfile.txt`:
```ini
   [requires]
   boost/1.84.0
   sqlite3/3.45.0
   spdlog/1.13.0
   mosquitto/2.0.18

   [generators]
   CMakeDeps
   CMakeToolchain

   [options]
   boost:shared=False
```

2. Install dependencies:
```bash
   conan install . --output-folder=bin --build=missing
```

3. Build with CMake using Conan-generated toolchain:
```bash
   cd bin
   cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
   cmake --build .
```

## Docker integration
- Conan is installed in the build stage of each Dockerfile
- Dependencies are installed during Docker build
- Conan cache can be mounted as a volume to speed up builds
- Example Dockerfile pattern:
```dockerfile
  FROM alpine:latest AS build
  RUN apk add --no-cache python3 py3-pip cmake build-base
  RUN pip3 install conan
  COPY conanfile.txt /app/
  WORKDIR /app
  RUN conan install . --output-folder=bin --build=missing
  COPY . /app/
  RUN cd bin && cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake && make
```

## Conan profiles
- Use Conan profiles for different build configurations
- Default profile for Linux/GCC builds
- Custom profiles for cross-compilation if needed
- Profile example:
```ini
  [settings]
  os=Linux
  arch=x86_64
  compiler=gcc
  compiler.version=13
  compiler.libcxx=libstdc++11
  build_type=Release
```

# Distributed system principles
- All microservices (except Redis and broker) are stateless
- Idempotent operations required:
  - Repeated requests do not change system state beyond first application
  - Retry-safe network calls, DB operations, message delivery
- System must tolerate multiple points of failure:
  - Service crash or restart
  - Network partition
  - MQTT broker outage
  - Frontend disconnection
- Asynchronous retry and backoff strategies
- Stateless services allow replication and failover

# Microservices overview

## 1. auth-service
- SSH key authentication
- JWT issuance and validation
- JWT stored in Redis with TTL 15 minutes
- Stateless except Redis storage
- Idempotent login operations

## 2. user-service
- Manages user profile data (non-sensitive)
- Requires valid JWT
- Stateless, async, multi-threaded
- Idempotent updates

## 3. topic-service
- Business entity: Topic
  - Each user can create a topic
  - Topic has a name, slogan (moto), participants
  - Only owner can change moto or delete the topic
  - Other users can post messages in the topic
- Publishes/subscribes via MQTT
- Stateless, async
- Idempotent message handling
- MQTT message payload includes JWT, request data, idempotency key

## 5. tui-frontend (server-side)
- SSH-based terminal UI (server-side)
- Communicates with MQTT broker
- Listens for events via MQTT topic subscription
- **Redis-backed session store**:
  - JWT for each user stored in Redis with TTL 15 minutes
  - Any frontend replica can validate JWT and serve SSH client
  - No sticky session required
- Asynchronous input, rendering, network events
- Idempotent command execution
- Minimal TUI commands (MVP):
  - `/send-message <topic> <message>`
  - `/change-moto <text>` (only if owner)
  - `/list-topics`
  - `/list-users`
- Future: replace MVP commands with full-featured, graphical server-side TUI interface

## 6. *.migrator microservices
- Standalone CLI for schema migrations
- Fully asynchronous
- Supports `up`, `down`, `to <version>`, `status`
- Idempotent operations

# Redis for server-side frontend scaling
- Server-side frontend requires state (JWT) per SSH session
- Redis acts as shared session store
- Mechanism:
  1. Auth-service validates SSH key, generates JWT, stores in Redis: `SETEX jwt:<user_id> 900 <signed_jwt>`
  2. TUI frontend replica retrieves JWT from Redis to validate session
  3. Multiple replicas can serve the same SSH client, reading JWT from Redis
  4. TTL ensures tokens expire automatically; active sessions can refresh TTL
- Result: frontend can be horizontally scaled, user experience remains seamless, no manual token management

# Internal communication (microservice-to-microservice)
- **MQTT**: async pub/sub for commands and events
- MQTT messages include:
  - JWT
  - Request data
  - Idempotency key

# External communication (client → system)
- **SSH**: only channel for connecting to server-side TUI
- TUI commands (MVP): `/send-message`, `/change-moto`, `/list-topics`, `/list-users`

# Fault-tolerance and resilience
- Services detect and recover from transient failures
- MQTT broker failures: retry connections, idempotent message handling
- Redis downtime: optionally fallback to re-authentication
- SSH client disconnections: can reconnect and resume session using Redis-backed JWT
- Database writes are transactional

# Deployment
- Each service containerized separately
- Docker Swarm stack:
  - auth-service + migrator
  - user-service + migrator
  - topic-service + migrator
  - tui-frontend (replicable)
  - MQTT broker
  - Redis
- Volumes for SQLite persistence
- Secrets for JWT keys

# C++ design rules
- Layers: API, core logic, repository, MQTT
- Async via Boost.Asio (coroutines or handler-based)
- HTTP via Boost.Beast
- Thread pools configurable
- Pimpl pattern for all public classes
- Error handling mandatory; all operations idempotent
- Graceful shutdown via signals
- Non-blocking logging via spdlog

# Non-goals
- No Telnet or plaintext protocols
- No monolithic or synchronous services
- No global shared state outside Redis and MQTT