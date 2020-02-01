#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include "Lua.hh"
#include "Registrar.hh"

// TODO: Serialize VM state to disk on halt / reload on init?

struct BoydScriptingState
{
    lua_State *L;

    BoydScriptingState()
        : L(nullptr)
    {
        BOYD_LOG(Debug, "Starting {}", LUA_RELEASE);
        L = luaL_newstate();
        luaL_openlibs(L);

        BOYD_LOG(Debug, "Registering Lua bindings");
        boyd::RegisterECS(L);
        boyd::RegisterAllComponents(L);

        BOYD_LOG(Debug, "Lua initialized");
    }
    ~BoydScriptingState()
    {
        BOYD_LOG(Debug, "Stopping Lua...");
        lua_close(L);
        L = nullptr;
        BOYD_LOG(Debug, "Lua stopped");
    }
};

inline static BoydScriptingState *GetState(void *state)
{
    return reinterpret_cast<BoydScriptingState *>(state);
}

extern "C" {
BOYD_API void *BoydInit_Scripting()
{
    BOYD_LOG(Info, "Starting scripting module");
    return new BoydScriptingState();
}

BOYD_API void BoydUpdate_Scripting(void *state)
{
}

BOYD_API void BoydHalt_Scripting(void *state)
{
    BOYD_LOG(Info, "Halting scripting module");
    delete GetState(state);
}
}