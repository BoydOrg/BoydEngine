#pragma once

#include "../Core/Platform.hh"
#include "../Core/Versioned.hh"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "../Core/Registrar.hh"

namespace boyd
{
namespace comp
{

/// A indexed, triangulated mesh.
struct BOYD_API Mesh
{
    struct Vertex
    {
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::vec3 normal{0.0f, 1.0f, 0.0f};
        glm::vec4 tintEmission{1.0f, 1.0f, 1.0f, 0.0f}; ///< RGB: tint, A: emission
        glm::vec2 texCoord{0.0f, 0.0f};
    };
    using Index = unsigned;
    enum Usage
    {
        Static = 0,  ///< Static; load once, render many times
        Dynamic = 1, ///< Dynamic; load frequently (but not really once per frame)
        Stream = 2,  ///< Stream; load/render once per frame, discard after use
    };

    struct Data
    {
        std::vector<Vertex> vertices{};
        std::vector<Index> indices{};
        Usage usage{Static};
    };
    Versioned<Data> data;

    // FIXME: Make so that the component must be changed to change `data`
    //       - i.e., that the user must assign_or_replace<>() it to get changes
    //         so that EnTT knows the mesh has changed

    /// Creates a new, empty mesh.
    Mesh()
        : data{Versioned<Data>::Make()}
    {
    }
    ~Mesh() = default;
};

} // namespace comp

template <typename TRegister>
struct Registrar<comp::Mesh, TRegister>
{
    static constexpr const char *TYPENAME = "Mesh";

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::Mesh>(TYPENAME)
            .template addConstructor<void(*)(void)>()
        .endClass();
        // clang-format on
    }
};
} // namespace boyd
