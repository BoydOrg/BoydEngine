#pragma once

#include "../Core/Platform.hh"
#include "../Scripting/Lua.hh"
#include <glm/glm.hpp>
#include <raylib.h>
#include <string>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Mesh
{
    std::string modelName;
    std::string textureName;

    /// Internally managed by Raylib, do not touch!
    Model model;
    Texture2D texture;
};

/*
template <>
struct ScriptRegistrar<Mesh>
{
    register()
};
*/

} // namespace comp
} // namespace boyd
