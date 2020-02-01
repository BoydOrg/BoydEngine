#pragma once

#include "BoydEngine.hh"

extern "C" {
#if defined(BOYD_LUA_LUAJIT)
#    include <luajit.h>
#else
#    include <lua.h>
#endif
#include <lauxlib.h>
#include <lualib.h>
}

#include <LuaBridge/LuaBridge.h>
