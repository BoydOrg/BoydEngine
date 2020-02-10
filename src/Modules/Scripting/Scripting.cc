#include "Scripting.hh"

#include "../../Components/LuaBehaviour.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include "LuaInternals.hh"
#include "Registrar.hh"

// TODO: Serialize VM state to disk on halt / reload on init?

namespace boyd
{

/// The path to the main script, to be execute automatically as the scripting module is started
static constexpr const char *MAIN_SCRIPT = "scripts/main.lua";

/// The name of the update function defined in the script; it is executed every update of the scripting module.
static constexpr const char *UPDATE_FUNC_NAME = "update";

/// The name of the update function defined in the script; it is executed when the scripting module is halted.
static constexpr const char *HALT_FUNC_NAME = "halt";

BoydScriptingState::BoydScriptingState()
    : L{nullptr}, compRefFactory{}
{
    BOYD_LOG(Debug, "Starting {}", LUA_RELEASE);
    L = luaL_newstate();
    luaL_openlibs(L);

    BOYD_LOG(Debug, "Registering Lua bindings");
    boyd::RegisterAllLuaTypes(this);

    BOYD_LOG(Debug, "Loading {}", MAIN_SCRIPT);
    luaL_dofile(L, MAIN_SCRIPT);
    const char *luaError = lua_tostring(L, -1);
    if(luaError)
    {
        BOYD_LOG(Error, "Lua error: {}", luaError);
    }

    BOYD_LOG(Debug, "Lua initialized");

    auto &registry = Boyd_GameState()->ecs;
    observer.connect(registry, entt::collector.group<comp::LuaBehaviour>());
}

BoydScriptingState::~BoydScriptingState()
{
    BOYD_LOG(Debug, "Stopping Lua...");
    // auto &registry = Boyd_GameState()->ecs();
    // auto view = registry.view<comp::LuaBehaviour>();

    // registry.destroy(view.begin(), view.end());

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

BOYD_API void BoydUpdate_Scripting(void *statePtr)
{
    auto *state = GetState(statePtr);

    // Call the update function in the main script, if any
    luabridge::LuaRef updateFunc = luabridge::getGlobal(state->L, boyd::UPDATE_FUNC_NAME);
    if(updateFunc.isFunction())
    {
        try
        {
            updateFunc();
        }
        catch(luabridge::LuaException &e)
        {
            BOYD_LOG(Error, "{}", e.what());
        }
    }
}

BOYD_API void BoydHalt_Scripting(void *statePtr)
{
    BOYD_LOG(Info, "Halting scripting module");

    auto *state = GetState(statePtr);
    // Call the halt function in the main script, if any.
    // (notice the inner scope to ensure `~LuaRef()` is called!)
    {
        luabridge::LuaRef haltFunc = luabridge::getGlobal(state->L, boyd::HALT_FUNC_NAME);
        if(haltFunc.isFunction())
        {
            haltFunc();
        }
    }

    delete GetState(state);
}
}