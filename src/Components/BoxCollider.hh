#pragma once

#include "../Core/Platform.hh"
#include "ColliderBase.hh"
#include <reactphysics3d.h>

namespace boyd
{

namespace comp
{

/// A wrapper on a box collider
struct BOYD_API BoxCollider : ColliderBase
{
    float x, y, z;
    BoxCollider(float x, float y, float z)
        : x{x}, y{y}, z{z}
    {
    }

    BoxCollider() = delete;
};
} // namespace comp
} // namespace boyd