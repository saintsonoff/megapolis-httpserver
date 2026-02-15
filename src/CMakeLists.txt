cmake_minimum_required(VERSION 4.0.0)

project(megapolis-httpserver)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(
    -Wno-psabi
    -Wall
    -Wextra
)


add_subdirectory(app)