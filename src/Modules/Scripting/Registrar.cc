#include "Registrar.hh"

#include "../../Core/GameState.hh"
#include <fmt/format.h>
#include <string>

#include "AllTypes.hh"

namespace boyd
{

void RegisterAllTypes(lua_State *L)
{
    using TRegister = luabridge::Namespace;
    auto ns = luabridge::getGlobalNamespace(L).beginNamespace(BOYD_NAMESPACE);

#define BOYD_REGISTER_TYPE(tname) ns = Registrar<tname, TRegister>::Register(ns);
    BOYD_REGISTER_ALLTYPES()

    ns.endNamespace();
}

// LuaEntity is a Lua table type that contains:
// - entity: entt::entity - The id of the entity

/// An entt::entity as an integer.
using EntityId = uint32_t;

/// LuaCFunction wrapper for EnTT's has() and get()
/// Lua args:
/// - self: LuaEntity
/// Lua returns:
/// - TComponent for the given entity, or null if component not found with the given name.
template <typename TComponent>
static int LuaGetComponent(lua_State *L)
{
    lua_settop(L, 1);                 // Discard all arguments except one
    luaL_checktype(L, 1, LUA_TTABLE); // Ensure first argument is a table
    lua_getfield(L, 1, "entity");     // Extract entity id
    EntityId entId = luabridge::Stack<EntityId>::get(L, -1);
    entt::entity ent{entId};

    auto *gameState = Boyd_GameState();
    if(gameState->ecs.has<TComponent>(ent))
    {
        luabridge::Stack<TComponent>::push(L, gameState->ecs.get<TComponent>(ent));
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

/// LuaCFunction wrapper for EnTT's assign_or_replace()
/// Lua args:
/// - self: LuaEntity
/// - comp: TComponent - The component to assign/replace for `self.entity`
/// Lua returns:
/// (void)
template <typename TComponent>
static int LuaSetComponent(lua_State *L)
{
    lua_settop(L, 2);                 // Discard all arguments except two
    luaL_checktype(L, 1, LUA_TTABLE); // Ensure first argument is a table
    lua_getfield(L, 1, "entity");     // Extract entity id
    EntityId entId = luabridge::Stack<EntityId>::get(L, -1);
    entt::entity ent{entId};

    auto *gameState = Boyd_GameState();
    gameState->ecs.assign_or_replace<TComponent>(ent, luabridge::Stack<TComponent>::get(L, 2));
    return 0;
}

/// LuaCFunction wrapper for EnTT's remove()
/// Lua args:
/// - self: LuaEntity
/// Lua returns:
/// (void)
template <typename TComponent>
static int LuaRemoveComponent(lua_State *L)
{
    lua_settop(L, 1);                 // Discard all arguments except first
    luaL_checktype(L, 1, LUA_TTABLE); // Ensure first argument is a table
    lua_getfield(L, 1, "entity");     // Extract entity id
    EntityId entId = luabridge::Stack<EntityId>::get(L, -1);
    entt::entity ent{entId};

    auto *gameState = Boyd_GameState();
    gameState->ecs.remove<TComponent>(ent);
    return 0;
}

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
    EntityId GetId() const
    {
        return EntityId(id);
    }

    /// LuaCFunction that returns a component reference for the `T` component slot
    /// attached to the given entity.
    /// Lua args:
    /// - self: LuaEntity
    /// - name: string
    /// Lua returns:
    /// - A LuaComponentRef<TComponent>() for the given entity, or null if component not found with the given name.
    int LuaComp(lua_State *L)
    {
        lua_settop(L, 2);                                             // Only accept 2 arguments
        LuaEntity self = luabridge::Stack<LuaEntity>::get(L, 1);      // self: LuaEntity
        std::string tname = luabridge::Stack<std::string>::get(L, 2); // tname: string

        // FIXME: Actually detect the type from `tname`!
        using TComponent = comp::Transform;

        // Create a new table specialized for getting/setting this type of component on this entity
        lua_createtable(L, 0, 4);

        luabridge::Stack<EntityId>::push(L, GetId());
        lua_setfield(L, -2, "entity");

        lua_pushcfunction(L, (&LuaGetComponent<TComponent>));
        lua_setfield(L, -2, "get");

        lua_pushcfunction(L, (&LuaSetComponent<TComponent>));
        lua_setfield(L, -2, "set");

        lua_pushcfunction(L, (&LuaRemoveComponent<TComponent>));
        lua_setfield(L, -2, "remove");

        // Return the table
        return 1;
    }

    /// Returns a pretty-printing of the entity.
    std::string ToString() const
    {
        return fmt::format("Entity({})", GetId());
    }
};

/// Lua function to create an EnTT entity.
static LuaEntity CreateLuaEntity()
{
    auto *gameState = Boyd_GameState();
    return LuaEntity{gameState->ecs.create()};
}
/// Lua function to get an EnTT entity by id.
static LuaEntity GetLuaEntity(entt::entity entity)
{
    auto *gameState = Boyd_GameState();
    return LuaEntity{gameState->ecs.valid(entity) ? entity : entt::null};
}

/// Lua function to destroy an EnTT entity.
static void DestroyLuaEntity(LuaEntity entity)
{
    auto *gameState = Boyd_GameState();
    gameState->ecs.destroy(entity.id);
}

void RegisterECS(lua_State *L)
{
    // clang-format off
    auto *gameState = Boyd_GameState();
    luabridge::getGlobalNamespace(L)
        .beginNamespace(BOYD_NAMESPACE)
            .beginClass<LuaEntity>("Entity")
                .addProperty("id", &LuaEntity::GetId)
                .addCFunction("comp", &LuaEntity::LuaComp)
                .addFunction("__tostring", &LuaEntity::ToString)
            .endClass()
            .beginNamespace("entity")
                .addFunction("create", &CreateLuaEntity)
                .addFunction("get", &GetLuaEntity)
                .addFunction("destroy", &DestroyLuaEntity)
            .endNamespace()
        .endNamespace();
    // clang-format on
}

} // namespace boyd