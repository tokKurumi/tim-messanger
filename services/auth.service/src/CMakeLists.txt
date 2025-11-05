cmake_minimum_required(VERSION 3.10)

project(auth.service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -static")

find_package(Boost REQUIRED)

add_executable(auth.service
    main.cpp
)

target_include_directories(auth.service PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(auth.service
    boost::boost
)

target_link_options(auth.service PRIVATE
    -static-libstdc++
    -static-libgcc
    -static
)