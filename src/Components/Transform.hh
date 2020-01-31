#pragma once

#include "../Core/Platform.hh"
#include "../Scripting/Lua.hh"
#include <glm/glm.hpp>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Transform
{
    ::Vector3 scale;
    ::Vector3 rotationAxis;
    ::Vector3 rotationAngle;
    ::Vector3 position;
    ::Vector3 skew;
};

/*
template <>
struct ScriptRegistrar<Transform>
{
    register()
};
*/

} // namespace comp
} // namespace boyd
