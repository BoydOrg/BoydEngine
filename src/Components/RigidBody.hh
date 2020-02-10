#pragma once

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"
#include <cmath>
#include <fmt/format.h>
#include <functional>

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

    RigidBody() = default;
    RigidBody(const RigidBody &) = default;
    RigidBody(RigidBody &&) = default;
    RigidBody &operator=(const RigidBody &) = default;
    RigidBody &operator=(RigidBody &&) = default;
};
} // namespace comp

template <typename TRegister>
struct Registrar<comp::RigidBody, TRegister>
{
    constexpr static const char *TYPENAME = "RigidBody";

    static float GetMass(const comp::RigidBody *self)
    {
        return self->mass;
    }

    static void SetMass(comp::RigidBody *self, float mass)
    {
        self->mass = mass;
    }

    static float GetRollingFriction(const comp::RigidBody *self)
    {
        return self->rollingFriction;
    }

    static void SetRollingFriction(comp::RigidBody *self, float rollingFriction)
    {
        self->rollingFriction = rollingFriction;
    }

    static float GetFriction(const comp::RigidBody *self)
    {
        return self->friction;
    }

    static void SetFriction(comp::RigidBody *self, float friction)
    {
        self->friction = friction;
    }

    static float GetBounciness(const comp::RigidBody *self)
    {
        return self->bounciness;
    }

    static void SetBounciness(comp::RigidBody *self, float bounciness)
    {
        self->bounciness = bounciness;
    }

    static bool GetGravity(const comp::RigidBody *self)
    {
        return self->enableGravity;
    }

    static void SetGravity(comp::RigidBody *self, bool gravity)
    {
        self->enableGravity = gravity;
    }

    static std::string ToString(const comp::RigidBody *self)
    {
        constexpr static const char *types[] = {
            "static",
            "kinematic",
            "dynamic"};

        // clang-format off
        return fmt::format(FMT_STRING("RigidBody (type={}, mass={}, rolling_friction={}, friction={}, bounciness={}, gravity={})"),
                types[static_cast<int>(self->type)],
                self->type == comp::RigidBody::DYNAMIC ? self->mass : INFINITY,
                self->rollingFriction,
                self->friction,
                self->bounciness,
                self->enableGravity
        );
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::RigidBody>(TYPENAME)
            .template addConstructor<void(*)(void)>()
            .addProperty("mass", GetMass, SetMass)
            .addProperty("rolling_friction", GetRollingFriction, SetRollingFriction)
            .addProperty("friction", GetFriction, SetFriction)
            .addProperty("bounciness", GetBounciness, SetBounciness)
            .addProperty("gravity", GetGravity, SetGravity)
            .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};
} // namespace boyd