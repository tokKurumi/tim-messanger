cmake_minimum_required(VERSION 3.10)

project(users.service LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

include_directories(include)

add_executable(users.service
    main.cpp
    services/print_service.cpp
)
