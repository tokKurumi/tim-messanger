cmake_minimum_required(VERSION 3.10)

project(prompt.service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -static")

find_package(Boost REQUIRED)
find_package(libssh REQUIRED)

add_executable(prompt.service
    main.cpp
    configuration/config.cpp
)

target_include_directories(prompt.service PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(prompt.service boost::boost)
target_link_libraries(prompt.service ssh::ssh)

target_link_options(prompt.service PRIVATE
    -static-libstdc++
    -static-libgcc
    -static
    -Wl,--allow-multiple-definition
)