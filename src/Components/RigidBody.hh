#pragma once

#include "../Core/Platform.hh"

namespace boyd
{

namespace comp
{

/// A wrapper on the various colliders
struct BOYD_API RigidBody
{
    enum RigidBodyType
    {
        STATIC,    /// Infinite mass, fixed object
        KINEMATIC, /// Infinite mass, object position is controlled by the physics engine
        DYNAMIC    /// Finite mass, object position is controlled by the physics engine
    } type;
    float mass; /// Measured in Kilograms

    /// -------------------------------------
    /// Material properties

    float rollingFriction; /// 0: no friction for Spheres and Capsules rolling on it, 1: high friction
    float friction;        /// 0: no friction for other objects sliding on it, 1: very high.
    float bounciness;      /// 0: not bouncy; 1: purely elastic.

    bool enableGravity{true};
};
} // namespace comp
} // namespace boyd