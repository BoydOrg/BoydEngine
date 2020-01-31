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
    glm::vec3 position;
    // Managed internally by Raylib, do not touch!
    ::Camera camera;
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