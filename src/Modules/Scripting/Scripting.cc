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
        boyd::RegisterAllTypes(L);
        boyd::RegisterECS(L);

        BOYD_LOG(Debug, "Lua initialized");

        // FIXME PAOLO - Debug only!!
        luaL_dostring(L, R"(
            io.stderr:write('Hello, Lua!\n')

            local ent = boyd.entity.create()
            io.stderr:write('Entity: ' .. tostring(ent) .. '\n')

            transfComp = ent:comp('Transform')
            io.stderr:write(tostring(transfComp) .. '\n')

            --for k, v in pairs(transfComp) do
            --    print(k .. ':' .. tostring(v) .. '\n')
            --end

            io.stderr:write('Initial transform = ' .. tostring(transfComp:get()) .. '\n')
            local newTransf = boyd.Transform():translated(10, 10, 10)
            transfComp:set(newTransf)
            io.stderr:write('New transform = ' .. tostring(transfComp:get()) .. '\n')
        )");
        const char *luaErr = lua_tostring(L, -1);
        BOYD_LOG(Debug, "Lua err: {}", luaErr ? luaErr : "<none>");
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