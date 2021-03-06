#pragma once

// TODO: Find a way to automate this(?!)

#include "AudioClip.hh"
#include "AudioSource.hh"
#include "BoxCollider.hh"
#include "Camera.hh"
#include "ComponentLoadRequest.hh"
#include "LuaBehaviour.hh"
#include "Mesh.hh"
#include "RigidBody.hh"
#include "String.hh"
#include "Transform.hh"

/// Massive macro containing all types to register to the scripting system
/// (as `BOYD_REGISTER_TYPE(<typename>)` entries)
#define BOYD_REGISTER_ALLTYPES()                         \
    BOYD_REGISTER_TYPE(boyd::comp::Transform)            \
    BOYD_REGISTER_TYPE(boyd::comp::String)               \
    BOYD_REGISTER_TYPE(boyd::comp::Camera)               \
    BOYD_REGISTER_TYPE(boyd::comp::ActiveCamera)         \
    BOYD_REGISTER_TYPE(boyd::comp::Mesh)                 \
    BOYD_REGISTER_TYPE(boyd::comp::AudioSource)          \
    BOYD_REGISTER_TYPE(boyd::comp::AudioClip)            \
    BOYD_REGISTER_TYPE(boyd::comp::ComponentLoadRequest) \
    BOYD_REGISTER_TYPE(boyd::comp::RigidBody)            \
    BOYD_REGISTER_TYPE(boyd::comp::BoxCollider)          \
    BOYD_REGISTER_TYPE(boyd::comp::LuaBehaviour)

// END
