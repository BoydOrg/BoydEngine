#include "Registrar.hh"

#include "../../Core/GameState.hh"
#include <fmt/format.h>
#include <string>

namespace boyd
{
// clang-format off

/// A Lua wrapper over an EnTT entity.
class LuaEntity
{
public:
    entt::entity id;

    explicit LuaEntity(entt::entity entity)
        : id{entity}
    {
    }
    ~LuaEntity()
    {
    }

    /// Returns the entity id as a raw integer.
    uint32_t getId() const
    {
        return uint32_t(id);
    }

    /// Returns a pretty-printing of the entity.
    std::string toString() const
    {
        return fmt::format("Entity({})", getId());
    }
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
                .addProperty("id", &LuaEntity::getId)
                .addFunction("__tostring", &LuaEntity::toString)
            .endClass()
            .beginNamespace("entity")
                .addFunction("create", &LuaCreateEntity)
                .addFunction("get", &LuaGetEntity)
                .addFunction("destroy", &LuaDestroyEntity)
            .endNamespace()
        .endNamespace();
}

} // namespace boyd
// -----------------------------------------------------------------------------
// TODO: Find a way to automate this (likely CMake generating a file list)!

#include "../../Components/Transform.hh"

namespace boyd
{

void RegisterAllComponents(lua_State *L)
{
    using TRegister = luabridge::Namespace;
    auto ns = luabridge::getGlobalNamespace(L).beginNamespace(BOYD_NAMESPACE);

    ns = Registrar<comp::Transform, TRegister>::Register(ns);

    ns.endNamespace();
}

// -----------------------------------------------------------------------------
// clang-format on
} // namespace boyd