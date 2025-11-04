cmake_minimum_required(VERSION 3.10)

project(auth.service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

# find_package(Boost REQUIRED COMPONENTS system thread)

add_executable(auth.service
    main.cpp
)

# target_link_libraries(auth.service
#     PRIVATE
#         Boost::system
#         Boost::thread
# )

target_link_options(auth.service PRIVATE -static-libstdc++ -static-libgcc)