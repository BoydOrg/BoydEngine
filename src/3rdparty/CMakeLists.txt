# Make sure all libraries are -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE YES)

# EnTT - https://github.com/skypjack/entt - License: MIT
add_subdirectory(entt/)

# fmtlib - https://github.com/fmtlib/fmt - License: modified MIT
add_subdirectory(fmt/)

# LuaBridge - https://github.com/vinniefalco/LuaBridge - License: modified MIT
set(LUABRIDGE_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(luabridge/)

include(Lua.cmake)

# glm - https://github.com/g-truc/glm - License: MIT + Happy Bunny License
add_subdirectory(glm/)

# OpenAL Soft - https://github.com/kcat/openal-soft/ - License: LGPLv2
set(ALSOFT_BACKEND_SDL2 OFF CACHE BOOL "" FORCE) # disable SDL2 mixer
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)     # Examples use SDL1 which cause us redefinition issues with raylib
add_subdirectory(openal-soft)

# Polyvox - https://bitbucket.org/volumesoffun/polyvox.git - License: zlib
set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_DOCS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(polyvox)
add_library(polyvox INTERFACE)
target_include_directories(polyvox INTERFACE ${PolyVoxHeaders_SOURCE_DIR})

# tinygltf - https://github.com/syoyo/tinygltf - License: MIT
set(TINYGLTF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/tinygltf/")

# glfw - https://github.com/glfw/glfw - License: zlib
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw/)
