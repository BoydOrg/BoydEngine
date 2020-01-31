#pragma once

#include "../Core/Platform.hh"
#include "../Scripting/Lua.hh"
#include <glm/glm.hpp>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Mesh
{
    glm::mat4 matrix;
};

template <>
struct ScriptRegistrar<Mesh>
{
    register()
};

} // namespace comp
} // namespace boyd
