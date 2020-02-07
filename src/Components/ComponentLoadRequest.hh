#pragma once

#include "../Core/Platform.hh"
#include <entt/entt.hpp>
#include <initializer_list>
#include <string>
#include <unordered_map>

namespace boyd
{
namespace comp
{

/// A load request for other components.
/// Attach one to the entity that you wish to load. When a request is received, its `ComponentLoadReq` is removed from
/// the parent
/// and the loaded asset is attached to the entity as a component (if loading didn't fail); when all requests have been
/// serviced (or errored out) the CompLoadReq is removed.
///
/// NOTE: Once a request is submitted (= attached to the ECS) it can't be changed - it will still go through even if the
///       component is removed / the entries are changed!
struct BOYD_API ComponentLoadRequest
{
    /// Shorthand to get the `entt::type_info<>` of a certain type.
    template <typename T>
    static constexpr ENTT_ID_TYPE TypeOf()
    {
        return entt::type_info<T>::id();
    }

    /// All <TypeOf(TAsset) -> filepath to load asset from> requests for asset loads for this entity.
    std::unordered_map<ENTT_ID_TYPE, std::string> requests;

    ComponentLoadRequest()
        : requests{}
    {
    }
    ComponentLoadRequest(const decltype(requests) &requests)
        : requests{requests}
    {
    }
    ComponentLoadRequest(std::initializer_list<typename decltype(requests)::value_type> requests)
        : requests{requests}
    {
    }
};

} // namespace comp
} // namespace boyd
