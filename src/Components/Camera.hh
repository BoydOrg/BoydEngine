#pragma once

#include "../Core/Platform.hh"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

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
} // namespace boyd
