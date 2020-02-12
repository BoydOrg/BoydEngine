#include "Scripting.hh"

#include "../../Components/ComponentLoadRequest.hh"
#include "../../Components/Gltf.hh"
#include "../../Components/LuaBehaviour.hh"
#include "../../Components/Transform.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include "LuaInternals.hh"
#include "Registrar.hh"

#include <BoydEngine.hh>

// TODO: Serialize VM state to disk on halt / reload on init?

namespace boyd
{

/// The path to the main script, to be execute automatically as the scripting module is started
static constexpr const char *MAIN_SCRIPT = BOYD_FS_PREFIX "scripts/main.lua";

/// LuaCFunction replacement for the builtin "print()" - logs to BOYD_LOG instead
/// Lua args:
/// - string*
/// Lua returns:
/// - (none)
static int LuaPrint(lua_State *L)
{
    static constexpr const char *TAB = "\t";
    static constexpr const char *NUL = "\0";

    int argc = lua_gettop(L);

    // First, convert all Lua arguments to a string
    static thread_local fmt::basic_memory_buffer<char, 512> buffer;
    buffer.clear();
    for(int i = 1; i <= argc; i++)
    {
        size_t argStrLen = 0;
        const char *argStr = luaL_tolstring(L, i, &argStrLen);
        buffer.append(argStr, argStr + argStrLen);
        // NOTE: Lua's `print()` uses tabs inbetween each argument!
        if(i < (argc - 1))
        {
            buffer.append(TAB, TAB + 1);
        }
    }
    buffer.append(NUL, NUL + 1);

    // Then, get which file and line the Lua script is printing from
    lua_Debug dbg;
    lua_getstack(L, 1, &dbg);
    lua_getinfo(L, "Sl", &dbg); // Fill specific fields in `dbg` - see Lua docs

    luabridge::LuaRef short_src = luabridge::getGlobal(L, boyd::GLOBAL_SCRIPT_IDENTIFIER);

    // Finally, log the message
    boyd::Log::instance().log(LogLevel::Debug, short_src.tostring().c_str(), dbg.currentline,
                              FMT_STRING("{}"), buffer.data());

    return 0;
}

BoydScriptingState::BoydScriptingState()
    : L{nullptr}, compRefFactory{}
{
    BOYD_LOG(Debug, "Starting {}", LUA_RELEASE);
    L = luaL_newstate();

    // Cherry-pick Lua libraries to open (excluding dangerous ones)
    // (See: linit.c)
    static constexpr const luaL_Reg LUA_LIBS[] = {
        {LUA_GNAME, luaopen_base},
        {LUA_LOADLIBNAME, luaopen_package},
        {LUA_COLIBNAME, luaopen_coroutine},
        {LUA_TABLIBNAME, luaopen_table},
        //{LUA_IOLIBNAME, luaopen_io},
        //{LUA_OSLIBNAME, luaopen_os},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {LUA_UTF8LIBNAME, luaopen_utf8},
#ifdef DEBUG
        {LUA_DBLIBNAME, luaopen_debug},
#endif
        {nullptr, nullptr},
    };
    for(auto *luaLib = LUA_LIBS; luaLib->name; luaLib++)
    {
        luaL_requiref(L, luaLib->name, luaLib->func, 1);
        lua_pop(L, 1);
    }

    // Replace Lua's `print()` with a custom one
    static constexpr const luaL_Reg LUA_PRINT_LIB[] = {
        {"print", LuaPrint},
        {nullptr, nullptr},
    };
    lua_getglobal(L, "_G");
    luaL_setfuncs(L, LUA_PRINT_LIB, 0);
    lua_pop(L, 1);

    BOYD_LOG(Debug, "Registering Lua bindings");
    boyd::RegisterAllLuaTypes(this);

    /*
    BOYD_LOG(Debug, "Loading {}", MAIN_SCRIPT);
    luaL_dofile(L, MAIN_SCRIPT);
    const char *luaError = lua_tostring(L, -1);
    if(luaError)
    {
        BOYD_LOG(Error, "Lua error: {}", luaError);
    }
    */

    BOYD_LOG(Debug, "Lua initialized");

    auto &registry = Boyd_GameState()->ecs;
    observer.connect(registry, entt::collector.group<comp::LuaBehaviour>());
}

BoydScriptingState::~BoydScriptingState()
{
    BOYD_LOG(Debug, "Stopping Lua...");
    auto &registry = Boyd_GameState()->ecs;
    auto view = registry.view<comp::LuaBehaviour>();

    registry.destroy(view.begin(), view.end());

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
    auto *state = new boyd::BoydScriptingState();
    auto &registry = Boyd_GameState()->ecs;

    auto root = registry.create();
    boyd::comp::ComponentLoadRequest luascript{{boyd::comp::ComponentLoadRequest::TypeOf<boyd::comp::LuaBehaviour>(),
                                                "scripts/main.lua"}};

    registry.assign<boyd::comp::ComponentLoadRequest>(root, std::move(luascript));

    auto testCube = registry.create();
    registry.assign<boyd::comp::Transform>(testCube, glm::translate(glm::identity<glm::mat4>(), glm::vec3{0.0f, 0.0f, -3.0f}));

    boyd::comp::ComponentLoadRequest cubeReq{
        {boyd::comp::ComponentLoadRequest::TypeOf<boyd::comp::Gltf>(), "assets/GLTF/TexturedCube.glb"},
    };
    registry.assign<boyd::comp::ComponentLoadRequest>(testCube, std::move(cubeReq));

    return state;
}

BOYD_API void BoydUpdate_Scripting(void *statePtr)
{
    auto *state = GetState(statePtr);
    auto &registry = Boyd_GameState()->ecs;
    for(auto entity : state->observer)
    {
        BOYD_LOG(Info, "Detected new script in ECS, building");
        auto &comp = registry.get<boyd::comp::LuaBehaviour>(entity);
        auto &internal = registry.assign_or_replace<boyd::comp::LuaInternals>(entity, comp, state->L, entity);
    }

    state->observer.clear();

    registry.view<boyd::comp::LuaInternals>().each([state](boyd::comp::LuaInternals &internals) {
        internals.Update(state->L);
    });
}

BOYD_API void BoydHalt_Scripting(void *statePtr)
{
    BOYD_LOG(Info, "Halting scripting module");

    auto *state = GetState(statePtr);
    /// Call the halt method of each LuaInternal
    Boyd_GameState()->ecs.view<boyd::comp::LuaInternals>().each([state](auto &l) { l.Halt(state->L); });

    delete GetState(state);
}
}