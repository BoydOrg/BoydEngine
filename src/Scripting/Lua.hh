#pragma once

#include "BoydEngine.hh"

extern "C" {
#if defined(BOYD_LUA_LUAJIT)
#    include <luajit.h>
#else
#    include <lua.h>
#endif
// clang-format off
#include <lualib.h>
#include <lauxlib.h>
// clang-format on
}

#include <LuaBridge/LuaBridge.h>
