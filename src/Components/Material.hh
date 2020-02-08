#pragma once

#include "../Core/Platform.hh"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include "Texture.hh"

namespace boyd
{
namespace comp
{

/// A `comp::Material`'s parameter; corresponds more or less to a shader uniform.
using MaterialParameter = std::variant<float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, comp::Texture>;

/// A graphics material.
/// Attach this to an entity with a Mesh and a Transform to make it renderable.
struct BOYD_API Material
{
    /// The parameters set in this material
    std::unordered_map<std::string, MaterialParameter> parameters;

    Material()
        : parameters{}
    {
    }

    Material(std::initializer_list<decltype(parameters)::value_type> parameters)
        : parameters{parameters}
    {
    }
};

} // namespace comp
} // namespace boyd