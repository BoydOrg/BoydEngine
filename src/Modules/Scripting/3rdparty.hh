#pragma once

#include "Lua.hh"

namespace boyd
{
/// Register glm classes and operators

void RegisterGLM(luabridge::Namespace &ns);
} // namespace boyd