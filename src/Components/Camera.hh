#pragma once

#include <entt/entt.hpp>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"

namespace boyd
{
namespace comp
{

/// A 3D camera (pretty much a projection matrix).
/// Attach it to an entity with a transform to move it!
struct BOYD_API Camera
{
    enum Mode
    {
        Persp = 0,
        Ortho = 1,
    } mode;

    float fov;                      ///< Horizontal FOV (in radians) for Persp cameras.
    float left, right, bottom, top; ///< Left, right, bottom, top for Ortho cameras.
    float zNear, zFar;              ///< Near and far clipping planes.
                                    ///  Set to +/-infinity on Ortho cameras to get an infinite viewing volume.

    /// Creates a perspective camera. fov is the horizontal FoV *in degrees*.
    static Camera Perspective(float fov, float zNear = 0.1f, float zFar = 1000.0f)
    {
        return {Persp, fov, 0.0f, 0.0f, 0.0f, 0.0f, zNear, zFar};
    }

    /// Creates a orthographic camera.
    static Camera Orthographic(float left, float right, float bottom, float top,
                               float zNear = -std::numeric_limits<float>::infinity(),
                               float zFar = std::numeric_limits<float>::infinity())
    {
        return {Ortho, 0.0f, left, right, bottom, top, zNear, zFar};
    }
};

/// Marks a Camera as the currently-active one.
/// Attach it to the same entity with the Camera to be made active.
using ActiveCamera = entt::tag<"ActiveCamera"_hs>;

} // namespace comp

template <typename TRegister>
struct Registrar<comp::Camera, TRegister>
{
    static constexpr const char *TYPENAME = "Camera";

    /// Creates a perspective camera. fov is the horizontal FoV *in degrees*.
    static comp::Camera Perspective(const comp::Camera *self, float fov, float zNear = 0.1f, float zFar = 1000.0f)
    {
        (void)self;
        return {comp::Camera::Persp, fov, 0.0f, 0.0f, 0.0f, 0.0f, zNear, zFar};
    }

    /// Creates a orthographic camera.
    static comp::Camera Orthographic(comp::Camera *self, float left, float right, float bottom, float top,
                                     float zNear = -std::numeric_limits<float>::infinity(),
                                     float zFar = std::numeric_limits<float>::infinity())
    {
        (void)self;
        return {comp::Camera::Ortho, 0.0f, left, right, bottom, top, zNear, zFar};
    }

    static std::string ToString(const comp::Camera *self)
    {
        return fmt::format(FMT_STRING("{} Camera"), self->mode ? "Perspective" : "Orthographic");
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::Camera>(TYPENAME)
            .template addConstructor<void(*)(void)>()
            .addFunction("perspective", Perspective)
            .addFunction("ortographic", Orthographic)
            .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};

template <typename TRegister>
struct Registrar<comp::ActiveCamera, TRegister>
{
    static constexpr const char *TYPENAME = "ActiveCamera";

    static std::string ToString(const comp::ActiveCamera *self)
    {
        (void)self;
        return "ActiveCamera";
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::ActiveCamera>(TYPENAME)
            .template addConstructor<void(*)(void)>()
            .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};

} // namespace boyd
