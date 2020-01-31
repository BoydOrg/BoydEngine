#pragma once

#include "../Core/Platform.hh"
#include "../Scripting/Lua.hh"
#include <glm/glm.hpp>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Camera
{
    ::Camera camera{0};
};

/*
template <>
struct ScriptRegistrar<Camera>
{
    register()
};
*/

} // namespace comp
} // namespace boyd