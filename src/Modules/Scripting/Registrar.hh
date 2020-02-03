#pragma once

#include <unordered_map>

#include "../../Core/GameState.hh"
#include "../../Core/Registrar.hh"
#include "Lua.hh"

namespace boyd
{

// The namespace name for the Lua Boyd API.
static constexpr const char *BOYD_NAMESPACE = "boyd";

/// Registers all known types (& their respective `ComponentRef<Type>`) for use into the given Lua state.
/// (They will all become available to EnTT for use as components.)
void RegisterAllTypes(lua_State *L);

/// Registers EnTT for use in Lua.
/// WARNING: Must be done after `RegisterAllTypes`!
void RegisterECS(lua_State *L);

} // namespace boyd