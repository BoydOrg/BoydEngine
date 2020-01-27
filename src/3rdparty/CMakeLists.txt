# Make sure all libraries are -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE YES)

# EnTT - https://github.com/skypjack/entt - License: MIT
add_subdirectory(entt/)

# Raylib - https://github.com/raysan5/raylib - License: BSD
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # avoid building examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)    # or games
add_subdirectory(raylib/)

# fmtlib - https://github.com/fmtlib/fmt - License: modified MIT
add_subdirectory(fmt/)

# LuaBridge - https://github.com/vinniefalco/LuaBridge - License: modified MIT
set(LUABRIDGE_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(luabridge/)
