cmake_minimum_required(VERSION 3.31)

project(megapolis-httpserver)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(
    -Wno-psabi
    -Wall
    -Wextra
)

find_package(Boost REQUIRED)
message(STATUS "Boost version: ${Boost_VERSION}")

add_subdirectory(root_checker)
add_subdirectory(utility)
add_subdirectory(handler)
add_subdirectory(router)
add_subdirectory(server)
add_subdirectory(app)