#pragma once

#include "../Core/Platform.hh"
#include "../Scripting/Lua.hh"
#include <glm/glm.hpp>
#include <raylib.h>

using RaylibCamera = Camera;

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Camera
{
    RaylibCamera camera{0};

    Camera(Vector3 position, float fovy, CameraType type)
    {
        camera.position = position;
        camera.up = (Vector3){0.0f, 1.0f, 0.0f};
        camera.target = (Vector3){0.0f, 0.0f, 0.0f};
        camera.fovy = fovy;
        camera.type = type;

        SetCameraMode(camera, CAMERA_FIRST_PERSON);
    }
};

/*
template <>
struct ScriptRegistrar<Camera>
{
    register()
};
*/

} // namespace comp
} // namespace boyd