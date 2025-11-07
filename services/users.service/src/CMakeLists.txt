cmake_minimum_required(VERSION 3.10)

project(users.service LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

add_executable(users.service
    main.cpp
    services/print_service.cpp
)

target_include_directories(users.service PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(users.service
    boost::boost
)
