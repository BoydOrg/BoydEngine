#pragma once

#include "Lua.hh"
#include "Scripting.hh"

namespace boyd
{

/// Registrs EnTT for use with Lua.
/// Then registers all known types (as enumerated in "AllTypes.hh") as follows:
/// - Registers their type into the given Lua state (via `Registrar<T>`; lua namespace `${BOYD_NAMESPACE}`)
/// - Registers their type id for use by Lua scripts (lua namespace `${BOYD_NAMESPACE}.comp`)
/// - Registers a <type id -> lua_CFunc function that returns Lua ComponentRef> for each type into `compFactory`
void RegisterAllLuaTypes(BoydScriptingState *state);

} // namespace boyd