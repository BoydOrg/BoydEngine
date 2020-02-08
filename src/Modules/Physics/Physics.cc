#include "../../Components/BoxCollider.hh"
#include "../../Components/ColliderInternals.hh"
#include "../../Components/RigidBody.hh"
#include "../../Components/Transform.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include <entt/entt.hpp>
#include <reactphysics3d.h>

using namespace reactphysics3d;

using namespace std;

struct BoydPhysicsState
{

    DynamicsWorld *world;
    entt::observer entt_Colliders[1];
    float timeStep;

    BoydPhysicsState(entt::registry &registry)
    {
        world = new DynamicsWorld(rp3d::Vector3{0.0, 9.81, 0.0});
        RegisterCollider<comp::BoxCollider>(registry, BOX_COLLIDER);

        /// TODO: Add the other colliders
        timeStep = 1.0f / 60.0f;
    }

    /// Register a collider into an observer.
    template <typename Collider>
    void RegisterCollider(entt::registry &registry, boyd::Collider type)
    {
        observer.connect(registry, entt::collector<comp::RigidBody, T> (entt::exclude<comp::ColliderInternals<Collider>>());
    }

    ~BoydPhysicsState()
    {
        delete world;
    }
};

/// Update the transform of a non-static rigid body.
/// This method is templetized because we do not know which collider is used. Luckily it's only 4 of them ...
template <typename Collider>
void UpdateTransform(entt::entity entity, entt::registry &registry, boyd::comp::RigidBody &rigidBody, boyd::comp::Transform &transform)
{
    using Internals = comp::ColliderInternals<Collider>;
    if(registry.has<Internals>())
    {
        registry.get<Internals>(entity).UpdateTransform(transform);
    }
}

inline BoydPhysicsState *GetState(void *state)
{
    return reinterpret_cast<BoydPhysicsState *>(state);
}

using namespace boyd;

extern "C" {
BOYD_API void *BoydInit_Physics(void)
{
    BOYD_LOG(Info, "Started physics module");
    return new BoydPhysicsState(Boyd_GameState()->ecs);
}
BOYD_API void BoydUpdate_Physics(void *state)
{
    auto *physicsState = GetState(state);
    auto &registry = Boyd_GameState()->ecs;
    auto rigidBodiesView = registry.view<comp::RigidBody, comp::Transform>();
    auto boxCollidersView = registry.view<comp::BoxCollider>();

    // Iterate over all the new/updated Rigid
    for(auto entity : physicsState->entt_Colliders[BOX_COLLIDER])
    {
        auto &rigidBody = rigidBodiesView.get<comp::RigidBody>(entity);
        auto &transform = rigidBodiesView.get<comp::Transform>(entity);
        auto &boxCollider = boxCollidersView.get(entity);
        registry.assign_or_replace<comp::ColliderInternals<comp::BoxCollider>>(entity,
                                                                               boxCollider,
                                                                               physicsState->world,
                                                                               rigidBody, transform);
    }

    /// TODO: add the other colliders
    physicsState->world->update(physicsState->timeStep);

    /// Copy the transforms
    for(auto entity : rigidBodiesView)
    {
        auto &rigidBody = rigidBodiesView.get<comp::RigidBody>(entity);
        auto &transform = rigidBodiesView.get<comp::Transform>(entity);

        if(rigidBody.type != comp::RigidBody::STATIC)
        {
            UpdateTransform<boyd::comp::BoxCollider>(entity, registry, rigidBody, transform);
        }
    }
}

BOYD_API void BoydHalt_Physics(void *state)
{
    auto &registry = Boyd_GameState()->ecs;
    auto *physicsState = GetState(state);

    auto boxColliderInternals = registry.view<comp::ColliderInternals<comp::BoxCollider>>();
    /// TODO: add the other colliders here

    // Destroy them!
    registry.destroy(boxColliderInternals.begin(), boxColliderInternals.end());
}
}