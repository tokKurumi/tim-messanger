# Services

This directory contains the microservices used by the tim-messanger project. Each service is built as a small, focused C++ microservice (C++20) and is intended to be run in a container.

Short list of services and their purpose:

-   `prompt.service` — SSH entrypoint and TUI frontend. Accepts SSH connections from users, collects credentials, and forwards requests to the auth service over MQTT. Provides the user-facing TUI.
-   `auth.service` — Authentication service. Validates username/password (or token) and issues JWT tokens used for identity and authorization between services.
-   `users.service` — User profile management. Stores non-sensitive user metadata and exposes simple CRUD operations (requires a valid JWT).
-   `topic.service` — Topic/message service. Manages topics, participants, and messages; publishes/subscribes events via MQTT. Owners can update topic metadata.
-   `migrator.service` — Database migrator CLI. Used to run schema migrations (up/down/to/status) for each service's SQLite database.

Notes:

-   Services are experimental and under active development. Check each service subfolder for a local `README.md` or build instructions (`conanfile`, `CMakeLists.txt`, `Dockerfile`).
-   Not all services may be fully implemented yet; some are placeholders or work-in-progress.
