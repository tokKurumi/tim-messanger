cmake_minimum_required(VERSION 3.10)

project(prompt.service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)
find_package(libssh REQUIRED)
find_package(spdlog REQUIRED)

add_executable(prompt.service
    main.cpp
    configuration/config.cpp
    services/executor_service.cpp
    services/runner_service.cpp
    services/ssh_listener_service.cpp
    services/ssh_session.cpp
)

target_include_directories(prompt.service PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(prompt.service PRIVATE
    boost::boost
    ssh::ssh
    spdlog::spdlog
)
