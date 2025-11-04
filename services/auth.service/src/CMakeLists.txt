cmake_minimum_required(VERSION 3.10)

project(auth.service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -static")

find_package(Boost REQUIRED COMPONENTS json)

add_executable(auth.service
    main.cpp
)

target_link_libraries(auth.service Boost::json)

target_link_options(auth.service PRIVATE -static-libstdc++ -static-libgcc)