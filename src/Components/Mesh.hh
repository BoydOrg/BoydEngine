#pragma once

#include "../Core/Platform.hh"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace boyd
{
namespace comp
{

/// A indexed, triangulated mesh.
struct BOYD_API Mesh
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 tintEmission; ///< RGB: tint, A: emission
        glm::vec2 texCoord;
    };
    using Index = unsigned;

    struct Data
    {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
    };
    std::shared_ptr<Data> data;

    /// Creates a new, empty mesh.
    Mesh()
        : data{std::make_shared<Data>()}
    {
    }
    ~Mesh() = default;
};

} // namespace comp
} // namespace boyd
