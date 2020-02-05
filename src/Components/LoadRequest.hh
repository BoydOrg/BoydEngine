#pragma once

#include "../Core/Platform.hh"
#include <entt/entt.hpp>
#include <string>

namespace boyd
{
namespace comp
{

/// A load request for another component.
/// Attach one to the entity that you wish to load; when the asset is loaded, the `LoadRequest` component will be removed
/// and the loaded component will be attached to the entity.
struct BOYD_API LoadRequest
{
    /// Shorthand to get the `entt::type_info<>` of a certain type.
    template <typename T>
    static constexpr ENTT_ID_TYPE TypeOf()
    {
        return entt::type_info<T>::id();
    }

    std::string filepath;       ///< Path to the file from which to load the resource.
    ENTT_ID_TYPE componentType; ///< The type of the asset (= EnTT component) to load.
};

} // namespace comp
} // namespace boyd
