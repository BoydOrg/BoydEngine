#pragma once

#include "../Core/Platform.hh"
#include "BoxCollider.hh"
#include "ColliderBase.hh"
#include "RigidBody.hh"
#include "Transform.hh"
#include <memory>
#include <reactphysics3d.h>
#include <type_traits>
#include <variant>

namespace boyd
{

enum Collider
{
    BOX_COLLIDER,
    SPHERE_COLLIDER,
    CAPSULE_COLLIDER,
    MESH_COLLIDER
};

constexpr int NUM_COLLIDERS = 4;

namespace comp
{

rp3d::CollisionShape *GetCollider(const boyd::comp::BoxCollider &comp)
{
    return new rp3d::BoxShape(rp3d::Vector3{comp.x, comp.y, comp.z});
}

template <typename ColliderType>
struct BOYD_API ColliderInternals
{
    /// A link to the World instance.
    /// Note: every time the physics is reloaded, this field will change.
    /// The reason why it is kept here is for convenience mainly, as manually destroying the handlers
    /// below is hairy, and hopefully no physics engine should make thread/library-wise contexts like OpenAL...
    std::shared_ptr<reactphysics3d::DynamicsWorld> world;

    /// The handler of the rigid body
    rp3d::RigidBody *rigidBodyHandler;
    /// The handler of the collider
    rp3d::CollisionShape *colliderHandler;
    /// The handler of the proxy shape collider (called Proxy in rp3d, aka a fixture in other physics engines)
    rp3d::ProxyShape *proxyShape;

    /// Initialize an internal.
    ColliderInternals(reactphysics3d::DynamicsWorld *world,
                      ColliderType &collider, boyd::comp::RigidBody &rigidBody,
                      Transform &transform)
        : world{world}
    {
        // Because C++ is too stupid to support traits (upgrading to C++20 is not planned yet)
        static_assert(std::is_base_of<ColliderBase, ColliderType>(), "The given collider should subclass ColliderType");
        rp3d::Transform localTransform;
        localTransform.setFromOpenGL((float *)&transform.matrix);
        rigidBodyHandler = world->createRigidBody(localTransform);

        rp3d::BodyType bodyType = static_cast<rp3d::BodyType>(rigidBody.type);

        rigidBodyHandler->setType(bodyType);
        if(bodyType == rp3d::BodyType::DYNAMIC)
        {
            rigidBodyHandler->setMass(rigidBody.mass);
        }
        rigidBodyHandler->enableGravity(rigidBody.enableGravity);

        rp3d::Material &material = rigidBodyHandler->getMaterial();

        material.setBounciness(rigidBody.bounciness);
        material.setFrictionCoefficient(rigidBody.friction);
        material.setRollingResistance(rigidBody.rollingFriction);

        colliderHandler = GetCollider(collider);
        proxyShape = rigidBodyHandler->addCollisionShape(colliderHandler, localTransform, rigidBody.mass);
    }

    /// Empty internals are useless, and colliders should not be shared.
    ColliderInternals() = delete;
    ColliderInternals(const ColliderInternals &) = delete;
    ColliderInternals &operator=(const ColliderInternals &) = delete;

    ColliderInternals(ColliderInternals &&) = default;
    ColliderInternals &operator=(ColliderInternals &&) = default;

    ~ColliderInternals()
    {
        // Delete the collider
        delete colliderHandler;
        // ... then the proxy shape
        rigidBodyHandler->removeCollisionShape(proxyShape);
        // ... then the rigid body
        world->destroyRigidBody(rigidBodyHandler);

        colliderHandler = nullptr;
        rigidBodyHandler = nullptr;
    }

    void UpdateTransform(boyd::comp::Transform &transform)
    {
        rp3d::Transform temp = rigidBodyHandler->getTransform();
        temp.getOpenGLMatrix((float *)&transform.matrix);
    }
};
} // namespace comp
} // namespace boyd