cmake_minimum_required(VERSION 3.10)

project(users.service C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(users.service
    main.c
    services/print_service.c
)
