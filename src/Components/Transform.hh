#pragma once

#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <vector>

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"
#include "../Core/Utils.hh"
#include "../Debug/Log.hh"

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Transform
{
    glm::mat4 matrix;

    Transform(const glm::mat4 &matrix = glm::identity<glm::mat4>())
        : matrix{matrix}
    {
    }
};

} // namespace comp

template <typename TRegister>
struct Registrar<comp::Transform, TRegister>
{
    static constexpr const char *TYPENAME = "Transform";

    static comp::Transform Translated(comp::Transform *self, glm::vec3 *translationVector)
    {
        CheckNull(self, translationVector);
        return {glm::translate(self->matrix, *translationVector)};
    }

    static comp::Transform Rotated(comp::Transform *self, float degrees, float axisX, float axisY, float axisZ)
    {
        CheckNull(self);
        return {glm::rotate(self->matrix, glm::radians(degrees), glm::vec3{axisX, axisY, axisZ})};
    }

    static comp::Transform Scaled(comp::Transform *self, float x, float y, float z)
    {
        CheckNull(self);
        return {glm::scale(self->matrix, glm::vec3{x, y, z})};
    }

    // Get the position of a vector from model space to world space
    static glm::vec3 AbsolutePosition(comp::Transform *self, glm::vec3 *position)
    {
        CheckNull(self, position);
        return glm::vec3{self->matrix * glm::vec4{*position, 1.0f}};
    }

    // Get the position of a vector from world space to model space
    static glm::vec3 RelativePosition(comp::Transform *self, glm::vec3 *position)
    {
        CheckNull(self, position);
        return glm::vec3{glm::inverse(self->matrix) * glm::vec4{*position, 1.0f}};
    }

    static comp::Transform LookAt(comp::Transform *self, glm::vec3 *position, glm::vec3 *target, glm::vec3 *up)
    {
        CheckNull(self, position, target, up);
        return {glm::lookAt(*position, *target, *up)};
    }

    static std::string ToString(comp::Transform *self)
    {
        CheckNull(self);
        return fmt::format(FMT_STRING("Transform ({})"), glm::to_string(self->matrix));
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::Transform>(TYPENAME)
            .template addConstructor<void(*)(void)>()
            .addFunction("translated", Translated)
            .addFunction("rotated", Rotated)
            .addFunction("scaled", Scaled)
            .addFunction("absolute_position", AbsolutePosition)
            .addFunction("relative_position", RelativePosition)
            .addFunction("look_at", LookAt)
            .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};

} // namespace boyd