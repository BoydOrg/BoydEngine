#pragma once

#include "../../Components/LuaBehaviour.hh"
#include "../../Debug/Log.hh"
#include "Lua.hh"

#include <chrono>
#include <entt/entt.hpp>
#include <fmt/format.h>
#include <string>

using namespace std::chrono;

namespace boyd
{
/// The name of the global variable that stores the path name.
/// ENRICO: This is pretty much hacky, and my original intention was to push a string like
/// "@pathname" in the stack before the code chunk so that debug.short_src would have been overwritten
/// but Lua does black magic(tm) and ignores that. Unfortunately I did not have the time and will to investigate.
static constexpr const char *GLOBAL_SCRIPT_IDENTIFIER = "_boyd_script_path";

/// The name of the update function defined in the script; it is executed every update of the scripting module.
static constexpr const char *UPDATE_FUNC_NAME = "update";

/// The name of the update function defined in the script; it is executed when the scripting module is halted.
static constexpr const char *HALT_FUNC_NAME = "halt";

namespace comp
{
struct BOYD_API LuaInternals
{
    std::string scriptPath;
    std::string scriptIdentifier;

    high_resolution_clock::time_point sleepTime;
    milliseconds sleepAmount;

    LuaInternals(const boyd::comp::LuaBehaviour &behaviour, lua_State *L, entt::entity scriptref)
        : scriptPath{behaviour.description}, scriptIdentifier{fmt::format(FMT_STRING("{}"), scriptref)}
    {
        //lua_pushfstring(L, "@%s", scriptPath.c_str());
        //lua_pushfstring(L, "=stdin");
        //lua_pushstring(L, "test.lua");
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

            /// HACK(Enrico): push the string name so that it is available as a variable inside lua.
            lua_pushstring(L, scriptPath.c_str());
            lua_setglobal(L, GLOBAL_SCRIPT_IDENTIFIER);
            lua_pcall(L, 0, LUA_MULTRET, 0);

            break;
        case LUA_ERRSYNTAX:
            BOYD_LOG(Error, "Syntax error: {}", lua_tostring(L, -1));
            // pop the filepath name
            lua_pop(L, 1);
            break;
        default:
            BOYD_LOG(Error, "Unexpected error: {}", lua_tostring(L, -1));
            lua_pop(L, 1);
            break;
        }
    }

    /// Call the Halt method, if it exists
    void Update(lua_State *L)
    {
        // Retrieve the table containing the functions of the chunk
        lua_pushstring(L, scriptPath.c_str());
        lua_setglobal(L, GLOBAL_SCRIPT_IDENTIFIER);
        /// Try to call the update function, if any. Catch any error.
        try
        {
            lua_getfield(L, LUA_REGISTRYINDEX, scriptIdentifier.c_str());
            lua_getfield(L, -1, UPDATE_FUNC_NAME);
            lua_call(L, 0, 0);
        }
        catch(luabridge::LuaException &e)
        {
            BOYD_LOG(Error, "{}{}", scriptPath, e.what() + 31);
        }
    }

    // Call the halt method, if it exists
    void Halt(lua_State *L)
    {
        lua_pushstring(L, scriptPath.c_str());
        lua_setglobal(L, GLOBAL_SCRIPT_IDENTIFIER);
        /// Try to call the halt function, if any. Catch any error.
        try
        {
            lua_getfield(L, LUA_REGISTRYINDEX, scriptIdentifier.c_str());
            lua_getfield(L, -1, HALT_FUNC_NAME);
            lua_call(L, 0, 0);
        }
        catch(luabridge::LuaException &e)
        {
            BOYD_LOG(Error, "{}{}", scriptPath, e.what() + 31);
        }
    }
};
} // namespace comp

} // namespace boyd