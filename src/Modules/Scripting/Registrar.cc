#include "Registrar.hh"

#include "../../Core/GameState.hh"

// TODO: Find a way to automate this (likely CMake generating a file list)!
#include "../../Components/AudioSource.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Transform.hh"

namespace boyd
{
// clang-format off

/// A Lua wrapper over an EnTT entity.
class LuaEntity
{
public:
    explicit LuaEntity(entt::entity entity)
        : id{entity}
    {
    }
    ~LuaEntity()
    {
    }

    entt::entity id;
};

/// Lua function to create an EnTT entity.
static LuaEntity LuaCreateEntity(lua_State *L)
{
    auto *gameState = Boyd_GameState();
    return LuaEntity{gameState->ecs.create()};
}
/// Lua function to get an EnTT entity by id.
static LuaEntity LuaGetEntity(entt::entity entity, lua_State *L)
{
    auto *gameState = Boyd_GameState();
    return LuaEntity{gameState->ecs.valid(entity) ? entity : entt::null};
}

/// Lua function to destroy an EnTT entity.
static void LuaDestroyEntity(LuaEntity entity, lua_State *L)
{
    auto *gameState = Boyd_GameState();
    gameState->ecs.destroy(entity.id);
}

void RegisterECS(lua_State *L)
{
    auto *gameState = Boyd_GameState();
    luabridge::getGlobalNamespace(L)
        .beginNamespace(BOYD_NAMESPACE)
            .beginClass<LuaEntity>("Entity")
                .addData("id", &LuaEntity::id)
            .endClass()
            .beginNamespace("entity")
                .addFunction("create", &LuaCreateEntity)
                .addFunction("get", &LuaGetEntity)
                .addFunction("destroy", &LuaDestroyEntity)
            .endNamespace()
        .endNamespace();
}

// -----------------------------------------------------------------------------

void RegisterAllComponents(lua_State *L)
{
    // FIXME IMPLEMENT!
}

// -----------------------------------------------------------------------------

// clang-format on
} // namespace boyd