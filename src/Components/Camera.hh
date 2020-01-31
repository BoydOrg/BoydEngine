#pragma once

#include "../Core/Platform.hh"
#include "../Scripting/Lua.hh"
#include <glm/glm.hpp>
#include <raylib.h>

using RaylibCamera = Camera;

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Camera
{
    RaylibCamera camera{0};
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