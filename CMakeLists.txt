cmake_minimum_required(VERSION 3.10)

project(BoydEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(BoydEngine
    src/Main.cc
)

configure_file(src/BoydEngine.hh.in src/BoydEngine.hh)
target_include_directories(BoydEngine PUBLIC
    "${PROJECT_BINARY_DIR}"
)

