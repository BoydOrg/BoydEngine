# Make sure all libraries are -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE YES)

# EnTT - https://github.com/skypjack/entt - License: MIT
add_subdirectory(entt/)

# Raylib - https://github.com/raysan5/raylib - License: BSD
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # avoid building examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)    # or games
set(USE_AUDIO OFF CACHE BOOL "" FORCE)      # or the audio module as we already use OpenAL-soft
set(OSS_LIBRARY "" CACHE STRING "" FORCE)   # disable OSS (as audio is disabled)
add_subdirectory(raylib/)

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

# FreeALUT - https://github.com/vancegroup/freealut - License: LGPLv2
option(BUILD_EXAMPLES "" OFF)
option(BUILD_TESTS "" ON)


# Xiph's Flac - https://github.com/janbar/flac-cmake - based on xiph/flac - License: Xiph's License + LGPL + GPL + FDL
set(BUILD_DOCS OFF CACHE BOOL "" FORCE) # Disable docs generation
add_subdirectory(flac)

# Polyvox - https://bitbucket.org/volumesoffun/polyvox.git - License: zlib
set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_DOCS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(polyvox)
add_library(polyvox INTERFACE)
target_include_directories(polyvox INTERFACE ${PolyVoxHeaders_SOURCE_DIR})
