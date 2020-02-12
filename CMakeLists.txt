cmake_minimum_required(VERSION 3.10)

project(BoydEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

cmake_policy(SET CMP0074 NEW)  # Look for <pkgname>_ROOT variables

add_subdirectory(src/)