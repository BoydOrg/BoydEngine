#pragma once

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Transform
{
    glm::mat4 matrix;
};

} // namespace comp

template <typename TRegister>
struct Registrar<comp::Transform, TRegister>
{
    static comp::Transform Translated(comp::Transform *self, float x, float y, float z)
    {
        return {glm::translate(self->matrix, glm::vec3{x, y, z})};
    }
    static comp::Transform Rotated(comp::Transform *self, float degrees, float axisX, float axisY, float axisZ)
    {
        return {glm::rotate(self->matrix, glm::radians(degrees), glm::vec3{axisX, axisY, axisZ})};
    }
    static comp::Transform Scaled(const comp::Transform *self, float x, float y, float z)
    {
        return {glm::scale(self->matrix, glm::vec3{x, y, z})};
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::Transform>("Transform")
            .addFunction("translated", Translated)
            .addFunction("rotated", Rotated)
            .addFunction("scaled", Scaled)
        .endClass();
        // clang-format on
    }
};

} // namespace boyd
