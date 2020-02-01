#pragma once

#include "../Core/Platform.hh"
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

} // namespace comp
} // namespace boyd
