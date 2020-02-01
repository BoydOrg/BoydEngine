#pragma once

#include "../Core/Platform.hh"
#include "../Scripting/Lua.hh"
#include <glm/glm.hpp>

#include <entt/entt.hpp>
#include <raylib.h>
#include <string>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Mesh
{
    /// Internally managed by Raylib, do not touch!
    Model model;
    Texture2D texture;

    Mesh(std::string modelName, std::string textureName)
    {
        model = LoadModel(modelName.c_str());
        if(textureName.size())
        {
            texture = LoadTexture(textureName.c_str());
            SetMaterialTexture(&model.materials[0], MAP_DIFFUSE, texture);
        }
    }
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
