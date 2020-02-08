#include "Registrar.hh"

#include "../../Core/GameState.hh"
#include "../../Debug/Log.hh"
#include <fmt/format.h>
#include <string>
#include <unordered_map>

#include "../../Components/AllTypes.hh"

// NOTE: The "LuaEntity" mentioned below is a Lua table type that contains:
// - entity: boyd::EntityId - The id of the entity

namespace boyd
{

// The namespace name for the Lua Boyd API.
static constexpr const char *BOYD_NAMESPACE = "boyd";

/// The key the identifies the pointer to the current `BoydScriptingState` into the Lua Registry of its `lua_State`.
static constexpr const char *STATE_LUAREGISTRY_KEY = "_boydState";

/// Put a pointer to `state` into the Lua Registry.
inline static void SetLuaScriptingState(lua_State *L, BoydScriptingState *state)
{
    lua_pushlightuserdata(L, state);
    lua_setfield(L, LUA_REGISTRYINDEX, STATE_LUAREGISTRY_KEY);
}

/// Retrieve the pointer to `state` from the Lua Registry, as saved by `SetLuaScriptingState()`.
inline static BoydScriptingState *GetLuaScriptingState(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, STATE_LUAREGISTRY_KEY);
    return reinterpret_cast<BoydScriptingState *>(lua_touserdata(L, -1));
}

// ---------------------------------------------------------------------------------------------------------------------

/// The underlying type of a `entt::entity`.
using EntityId = uint32_t;

/// LuaCFunction wrapper for EnTT's has() and get()
/// Lua args:
/// - self: LuaEntity
/// Lua returns:
/// - TComponent for the given entity, or null if component not found with the given name.
template <typename TComponent>
static int LuaGetComponent(lua_State *L)
{
    lua_settop(L, 1);                 // Discard all arguments except 1 (self)
    luaL_checktype(L, 1, LUA_TTABLE); // Ensure first argument is a table
    lua_getfield(L, 1, "entity");     // <-- Get `self.entity` --v
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
    lua_settop(L, 2);                 // Discard all arguments except 2 (self, comp)
    luaL_checktype(L, 1, LUA_TTABLE); // Ensure first argument is a table
    lua_getfield(L, 1, "entity");     // <-- Get `self.entity` --v
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
    lua_settop(L, 1);                 // Discard all arguments except 1 (self)
    luaL_checktype(L, 1, LUA_TTABLE); // Ensure first argument is a table
    lua_getfield(L, 1, "entity");     // <-- Get `self.entity` --v
    EntityId entId = luabridge::Stack<EntityId>::get(L, -1);
    entt::entity ent{entId};

    auto *gameState = Boyd_GameState();
    gameState->ecs.remove<TComponent>(ent);
    return 0;
}

// A LuaCFunction that returns a table specialized for manipulating `TComponent`s for a certain entity.
/// Lua args:
/// - self: LuaEntity
/// Lua returns:
/// - LuaComponentRef<TComponent, self>, or error if invalid component type
template <typename TComponent>
static int LuaCreateCompRef(lua_State *L)
{
    lua_settop(L, 1); // Discard all arguments except 1 (self)
    // TODO: Ensure [1] is a LuaEntity userdata?
    lua_getfield(L, 1, "id"); // <-- Get `self.id` --v
    EntityId entityId = luabridge::Stack<EntityId>::get(L, -1);

    // Create the table, fill it and return it
    lua_createtable(L, 0, 4);

    luabridge::Stack<EntityId>::push(L, entityId);
    lua_setfield(L, -2, "entity");

    lua_pushcfunction(L, (&LuaGetComponent<TComponent>));
    lua_setfield(L, -2, "get");

    lua_pushcfunction(L, (&LuaSetComponent<TComponent>));
    lua_setfield(L, -2, "set");

    lua_pushcfunction(L, (&LuaRemoveComponent<TComponent>));
    lua_setfield(L, -2, "remove");

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------

/// A Lua wrapper over an EnTT entity.
struct LuaEntity
{
    EntityId id;

    explicit LuaEntity(EntityId id)
        : id{id}
    {
    }

    /// LuaCFunction that creates a LuaEntity (& the respective EnTT entity) and returns it.
    /// Lua args:
    /// (none)
    /// Lua returns:
    /// - LuaEntity
    static int LuaCreateEntity(lua_State *L)
    {
        lua_settop(L, 0); // Ignore any arguments

        auto *gameState = Boyd_GameState();

        LuaEntity luaEntity{EntityId(gameState->ecs.create())};
        luabridge::Stack<LuaEntity>::push(L, luaEntity);
        return 1;
    }

    /// LuaCFunction to get an EnTT entity (as a LuaEntity) by id.
    /// Lua args:
    /// - id: EntityId
    /// Lua returns:
    /// - LuaEntity or (nil, string) on error
    static int LuaGetEntity(lua_State *L)
    {
        lua_settop(L, 1); // Discard all arguments except 1 (id)
        EntityId entityId = luabridge::Stack<EntityId>::get(L, 1);
        entt::entity entity{entityId};

        auto *gameState = Boyd_GameState();

        if(!gameState->ecs.valid(entity))
        {
            lua_pushnil(L);
            lua_pushstring(L, "Invalid entity");
            return 2;
        }

        LuaEntity luaEntity{entityId};
        luabridge::Stack<LuaEntity>::push(L, luaEntity);
        return 1;
    }

    /// LuaCFunction to destroy an EnTT entity by its id (if it exists)
    /// Lua args:
    /// - id: EntityId
    /// Lua returns:
    /// (none)
    static int LuaDestroyEntity(lua_State *L)
    {
        lua_settop(L, 1); // Discard all arguments except 1 (id)
        EntityId entityId = luabridge::Stack<EntityId>::get(L, 1);
        entt::entity entity{entityId};

        auto *gameState = Boyd_GameState();
        gameState->ecs.destroy(entity);
        return 0;
    }

    /// Checks whether the entity is valid or not.
    bool IsValid() const
    {
        auto *gameState = Boyd_GameState();
        return gameState->ecs.valid(entt::entity{id});
    }

    /// A LuaCFunction that returns a LuaComponentRef<> given the name of the component.
    /// Lua args:
    /// - self: LuaEntity
    /// - tname: string - The TYPENAME of the component (as defined in `boyd::Registrar<TComponent>::TYPENAME`)
    /// Lua returns:
    /// - LuaComponentRef<TComponent, self>, or (nil, <error string>) if invalid component typename
    int LuaGetCompRef(lua_State *L)
    {
        lua_settop(L, 2); // Discard all arguments except 2 (self, tname)
        // 1st arg: self

        luaL_checkstring(L, 2);                                          // Check that tname is indeed a string
        std::string tname = lua_tostring(L, 2);                          // Get tname
        ENTT_ID_TYPE typeId = entt::hashed_string::value(tname.c_str()); // Hash tname to get index into component ref factory

        // Get the `LuaComponentRef<T>(self)` constructor (from the factory method map)
        BoydScriptingState *state = GetLuaScriptingState(L);
        auto it = state->compRefFactory.find(typeId);
        if(it == state->compRefFactory.end())
        {
            lua_pushnil(L);
            luabridge::Stack<std::string>::push(L, fmt::format(FMT_STRING("Unknown type: `{}`"), tname));
            return 2;
        }

        // Call `LuaComponentRef<T>(self)`
        return it->second(L);
    }

    /// Returns a pretty-printing of the entity.
    std::string
    ToString() const
    {
        return fmt::format(FMT_STRING("Entity({})"), id);
    }
};

struct LuaInput
{
    static float getAxis(int index)
    {
        return Boyd_GameState()->Input.axes[index];
    }
};

// ---------------------------------------------------------------------------------------------------------------------
#ifdef DEBUG

/// Sanity check to prevent cryptic crashes: all types need unique TYPENAMEs
static bool AllLuaTypesAreUnique()
{
    using TRegister = luabridge::Namespace;

    std::unordered_map<std::string, unsigned> counts;
#    define BOYD_REGISTER_TYPE(tname) ++counts[Registrar<tname, TRegister>::TYPENAME];
    BOYD_REGISTER_ALLTYPES()
#    undef BOYD_REGISTER_TYPE

    bool allUnique = true;
    for(const auto &pair : counts)
    {
        if(pair.second > 1)
        {
            BOYD_LOG(Error, "Registrar: TYPENAME={} is not unique - it was registered {} times!", pair.first, pair.second);
            allUnique = false;
        }
    }
    return allUnique;
}
#endif

void RegisterAllLuaTypes(BoydScriptingState *state)
{
    // Add a pointer to `state` to its lua_State so that we can get it back from Lua functions
    SetLuaScriptingState(state->L, state);

    using TRegister = luabridge::Namespace;
    auto ns = luabridge::getGlobalNamespace(state->L).beginNamespace(BOYD_NAMESPACE);

#ifdef DEBUG
    /// Sanity check to prevent cryptic crashes: all types need unique TYPENAMEs
    if(!AllLuaTypesAreUnique())
    {
        BOYD_DEBUGGER_TRAP();
    }
#endif

    // Register all types to Lua and their CompRef factories to `compFactory`
#define BOYD_REGISTER_TYPE(tname)                   \
    ns = Registrar<tname, TRegister>::Register(ns); \
    state->compRefFactory[entt::hashed_string::value(Registrar<tname, TRegister>::TYPENAME)] = &LuaCreateCompRef<tname>;

    BOYD_REGISTER_ALLTYPES()

    // Then register `LuaEntity`
    // clang-format off
    ns = ns.beginClass<LuaEntity>("Entity")
        .addStaticCFunction("create", &LuaEntity::LuaCreateEntity)
        .addStaticCFunction("get", &LuaEntity::LuaGetEntity)
        .addStaticCFunction("destroy", &LuaEntity::LuaDestroyEntity)
        .addCFunction("comp", &LuaEntity::LuaGetCompRef)
        .addFunction("isvalid", &LuaEntity::IsValid)
        .addFunction("__tostring", &LuaEntity::ToString)
        .addProperty("id", &LuaEntity::id, false)
    .endClass();

    // clang-format on

    // Input bindings
    // clang-format off
    ns = ns.beginNamespace("Input")
        .addFunction("getAxis", &LuaInput::getAxis)
    .endNamespace();

    // clang-format on

    ns.endNamespace();
}

} // namespace boyd
