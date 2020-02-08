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
    int x, y, z;
    BoxCollider(int x, int y, int z)
        : x{x}, y{y}, z{z}
    {
    }

    BoxCollider() = delete;
};
} // namespace comp
} // namespace boyd