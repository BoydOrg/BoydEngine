#pragma once

#include "Lua.hh"

namespace boyd
{

// The namespace name for the Lua Boyd API.
static constexpr const char *BOYD_NAMESPACE = "boyd";

/// Registers EnTT for use in Lua.
void RegisterECS(lua_State *L);

/// Registers all known components into the given Lua state.
void RegisterAllComponents(lua_State *L);

} // namespace boyd