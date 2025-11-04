cmake_minimum_required(VERSION 3.10)

project(prompt.service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -static")

find_package(Boost REQUIRED COMPONENTS json)
find_package(libssh REQUIRED)

add_executable(prompt.service
    main.cpp
)

target_link_libraries(prompt.service Boost::json)
target_link_libraries(prompt.service ssh::ssh)

target_link_options(prompt.service PRIVATE -static-libstdc++ -static-libgcc)