# Make sure all libraries are -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE YES)

# EnTT - https://github.com/skypjack/entt - License: MIT
add_subdirectory(entt/)

# Raylib - https://github.com/raysan5/raylib - License: BSD
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # avoid building examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)    # or games
set(USE_AUDIO OFF CACHE BOOL "" FORCE)      # or the audio module as we already use OpenAL-soft
add_subdirectory(raylib/)

# fmtlib - https://github.com/fmtlib/fmt - License: modified MIT
add_subdirectory(fmt/)

# glm - https://github.com/g-truc/glm - License: MIT + Happy Bunny License
add_subdirectory(glm/)

# OpenAL Soft - https://github.com/kcat/openal-soft/ - License: LGPL
set(ALSOFT_BACKEND_SDL2 OFF CACHE BOOL "" FORCE) # disable SDL2 mixer
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)     # Examples use SDL1 which cause us redefinition issues with raylib
add_subdirectory(openal-soft)

# Tinywav - https://github.com/mhroth/tinywav - License: MIT
add_library(tinywav STATIC tinywav/tinywav.c)
target_include_directories(tinywav PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/tinywav")