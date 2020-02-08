#pragma once

namespace boyd
{
namespace comp
{
/// -- Do not use this component! Please use use of the many subclasses like BoxCollider, MeshCollider etc. --
/// Base structure for all colliders. It only serves the purpose to be inherited by the other colliders and provide
/// compile-time checks.
struct ColliderBase
{
};
} // namespace comp
} // namespace boyd