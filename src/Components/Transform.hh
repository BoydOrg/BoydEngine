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

    static std::string ToString(comp::Transform *self)
    {
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
            .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};

} // namespace boyd