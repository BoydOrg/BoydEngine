#pragma once

#include <entt/entt.hpp>
#include <unordered_map>

#include "Lua.hh"

namespace boyd
{

/// A map of <hashed `boyd::Registrar<T>::TYPENAME` -> Lua CFunctions that generates a ComponentRef<T> >
using LuaComponentRefFactory = std::unordered_map<ENTT_ID_TYPE, lua_CFunction>;

/// The state of the Scripting module.
struct BoydScriptingState
{
    lua_State *L;
    LuaComponentRefFactory compRefFactory;

    BoydScriptingState();
    ~BoydScriptingState();
};

} // namespace boyd