#pragma once

#include "../../Components/LuaBehaviour.hh"
#include "Lua.hh"

#include <chrono>
#include <fmt/format.h>
#include <string>

namespace boyd
{
namespace comp
{
struct BOYD_API LuaInternals
{
    std::string scriptIdentifier;
    bool withUpdate = true, withHalt = true;

    LuaInternals(const boyd::comp::LuaBehaviour &behaviour, lua_State *L, int scriptref)
        : scriptIdentifier{fmt::format(FMT_STRING("luascript#{}"), scriptref)}
    {
        switch(luaL_loadstring(L, behaviour.source.c_str()))
        {
        case LUA_OK:
            BOYD_LOG(Info, "Script loaded successfully");
            /// Based on https://stackoverflow.com/a/36408812
            // create _ENV tables
            lua_newtable(L);
            // create metatables
            lua_newtable(L);
            // Get the global table
            lua_getglobal(L, "_G");
            lua_setfield(L, -2, "__index");
            // Set global as the metatable
            lua_setmetatable(L, -2);
            // Push to the registry with a unique name
            lua_setfield(L, LUA_REGISTRYINDEX, scriptIdentifier.c_str());
            lua_getfield(L, LUA_REGISTRYINDEX, scriptIdentifier.c_str());
            // set the upvalue (_ENV)
            lua_setupvalue(L, 1, 1);
            lua_pcall(L, 0, LUA_MULTRET, 0);

            break;
        case LUA_ERRSYNTAX:
            BOYD_LOG(Error, "Syntax error: {}", lua_tostring(L, -1));
            L = nullptr;
            break;
        default:
            BOYD_LOG(Error, "Unexpected error: {}", lua_tostring(L, -1));
            L = nullptr;
            break;
        }
    }

    /// Call the Halt method, if it exists
    void Update(lua_State *L)
    {
        if(withUpdate)
        {
            //Retrieve the table containing the functions of the chunk
            lua_getfield(L, LUA_REGISTRYINDEX, scriptIdentifier.c_str());
            //Get the function we want to call
            lua_getfield(L, -1, "update");

            int type = lua_type(L, -1);

            if(!lua_isnil(L, -1))
            {
                //Call it
                lua_call(L, 0, 0);
            }
        }
    }

    // Call the halt method, if it exists
    void Halt(lua_State *L)
    {
        if(withHalt)
        {
            lua_getfield(L, LUA_REGISTRYINDEX, scriptIdentifier.c_str());
            lua_getfield(L, -1, "halt");
            lua_call(L, 0, 0);
        }
    }
};
} // namespace comp
} // namespace boyd