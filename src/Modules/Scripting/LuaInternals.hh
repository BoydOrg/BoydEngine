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

/// The name of the global variable that stores the entity of the script component.
static constexpr const char *GLOBAL_ENTITY_IDENTIFIER = "_boyd_entity";

/// The name of the update function defined in the script; it is executed every update of the scripting module.
static constexpr const char *UPDATE_FUNC_NAME = "update";

/// The name of the update function defined in the script; it is executed when the scripting module is halted.
static constexpr const char *HALT_FUNC_NAME = "halt";

using EntityId = uint32_t;

namespace comp
{
struct BOYD_API LuaInternals
{
    std::string scriptPath;
    EntityId entity;
    std::string scriptIdentifier;

    high_resolution_clock::time_point sleepTime{};
    milliseconds sleepAmount{milliseconds::zero()};

    LuaInternals(const boyd::comp::LuaBehaviour &behaviour, lua_State *L, entt::entity scriptref)
        : scriptPath{behaviour.description}, entity{EntityId(scriptref)}, scriptIdentifier{fmt::format(FMT_STRING("{}"), scriptref)}
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

            // PushGlobals(L);
            try
            {
                lua_call(L, 0, 0);
            }
            catch(luabridge::LuaException &e)
            {
                BOYD_LOG(Error, "{}", e.what());
            }

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

    ~LuaInternals()
    {
        BOYD_LOG(Info, "Goodbye");
    }

    /// Call the Halt method, if it exists
    void Update(lua_State *L)
    {
        /// If enough time has passed, continue the thread
        if(sleepAmount > milliseconds::zero() && duration_cast<milliseconds>(high_resolution_clock::now() - sleepTime) > sleepAmount)
            sleepAmount = milliseconds::zero();

        /// Else yield
        if(sleepAmount > milliseconds::zero())
            return;

        //PushGlobals(L);

        /// Try to call the update function, if any. Catch any error.
        try
        {
            lua_getfield(L, LUA_REGISTRYINDEX, scriptIdentifier.c_str());
            lua_getfield(L, -1, UPDATE_FUNC_NAME);
            if(lua_isnil(L, -1))
            {
                lua_Debug debug;
                lua_getstack(L, 1, &debug);
                lua_getinfo(L, "Sl", &debug);

                //__builtin_trap();
            }
#ifdef BOYD_LUA_JIT
            switch(::lua_resume(L, 0))
#else
            int nres[10]; // a conveniently large buffer
            switch(::lua_resume(L, nullptr, 0, nres))
#endif
            {
            case LUA_ERRRUN:
                BOYD_LOG(Error, "{} - {}", scriptPath, lua_tostring(L, -1));
                lua_pop(L, -1);
                lua_resetthread(L);
                break;
            }
        }
        catch(luabridge::LuaException &e)
        {
            BOYD_LOG(Error, "{} - {}", scriptPath, e.what());
        }
    }

    // Call the halt method, if it exists
    void Halt(lua_State *L)
    {
        PushGlobals(L);
        /// Try to call the halt function, if any. Catch any error.
        try
        {
            lua_getfield(L, LUA_REGISTRYINDEX, scriptIdentifier.c_str());
            lua_getfield(L, -1, HALT_FUNC_NAME);
            lua_call(L, 0, 0);
        }
        catch(luabridge::LuaException &e)
        {
            BOYD_LOG(Error, "{}{}", scriptPath, e.what());
        }
    }

private:
    /// HACK(Enrico): push the string name so that it is available as a variable inside lua.
    /// Also make the entity id available too.
    void PushGlobals(lua_State *L)
    {
        lua_pushstring(L, scriptPath.c_str());
        lua_setglobal(L, GLOBAL_SCRIPT_IDENTIFIER);
        lua_pushinteger(L, static_cast<uint32_t>(entity));
        lua_setglobal(L, GLOBAL_ENTITY_IDENTIFIER);
    }
};
} // namespace comp

} // namespace boyd