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
    // Start with a unitary matrix
    glm::mat4 matrix{1.0};
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
