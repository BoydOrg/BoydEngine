set(BOYD_LUA "Lua" CACHE STRING "What Lua runtime to use")
set_property(CACHE BOYD_LUA PROPERTY STRINGS Lua LuaJIT)

if(BOYD_LUA STREQUAL "Lua")
    # Lua 5.x - http://lua.org - License: MIT
    add_library(Lua STATIC lua/onelua.c)
    target_include_directories(Lua PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/lua/"
    )
    if(WIN32)
        target_compile_definitions(Lua PUBLIC -DLUA_WIN)
        set(LUA_PLATFORM -DLUA_WIN)
    elseif(UNIX AND APPLE)
        target_compile_definitions(Lua PUBLIC -DLUA_USE_MACOSX)
    elseif(UNIX)
        # Assume Linux...
        target_compile_definitions(Lua PUBLIC -DLUA_USE_LINUX)
        target_link_libraries(Lua PUBLIC m dl readline)
    endif()
elseif(BOYD_LUA STREQUAL "LuaJIT")
    # LuaJIT - http://luajit.org/download.html - License: MIT
    # CMake script: BSD3 licensed, https://github.com/OpenXRay/xray-16
    include(LuaJIT.cmake)
else()
    message(FATAL_ERROR "Unknown Lua runtime type!")
endif()
