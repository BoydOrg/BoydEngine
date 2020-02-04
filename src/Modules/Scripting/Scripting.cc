#include "Scripting.hh"

#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include "Registrar.hh"

// TODO: Serialize VM state to disk on halt / reload on init?

namespace boyd
{

/// The name of the main script to execute automatically as the scripting module is started.
static constexpr const char *AUTOEXEC_SCRIPT = "res/autoexec.lua";

BoydScriptingState::BoydScriptingState()
    : L{nullptr}, compRefFactory{}
{
    BOYD_LOG(Debug, "Starting {}", LUA_RELEASE);
    L = luaL_newstate();
    luaL_openlibs(L);

    BOYD_LOG(Debug, "Registering Lua bindings");
    boyd::RegisterAllLuaTypes(this);

    BOYD_LOG(Debug, "Loading {}", AUTOEXEC_SCRIPT);
    // FIXME luaL_loadfilex(L, AUTOEXEC_SCRIPT, "r");
    //const char *luaErr = lua_tostring(L, -1);
    //BOYD_LOG(Debug, "Lua err: {}", luaErr ? luaErr : "<none>");

    BOYD_LOG(Debug, "Lua initialized");
}

BoydScriptingState::~BoydScriptingState()
{
    BOYD_LOG(Debug, "Stopping Lua...");
    lua_close(L);
    L = nullptr;
    BOYD_LOG(Debug, "Lua stopped");
}

}; // namespace boyd

inline static boyd::BoydScriptingState *GetState(void *state)
{
    return reinterpret_cast<boyd::BoydScriptingState *>(state);
}

extern "C" {
BOYD_API void *BoydInit_Scripting()
{
    BOYD_LOG(Info, "Starting scripting module");
    return new boyd::BoydScriptingState();
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