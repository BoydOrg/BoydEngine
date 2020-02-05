#pragma once

#include <entt/entt.hpp>
#include <memory>

namespace boyd
{

struct LoadedAssetBase
{
    virtual ~LoadedAssetBase() = default;

    /// Assign(/replace) the loaded component to the EnTT entity `target`.
    virtual void AssignComponent(entt::registry &ecs, entt::entity target) = 0;
};

/// A response to a `LoadJob`, containing the loaded `TAsset`.
template <typename TAsset>
struct LoadedAsset : public LoadedAssetBase
{
    TAsset asset;

    LoadedAsset(TAsset &&asset)
        : asset{std::move(asset)}
    {
    }
    ~LoadedAsset() = default;

    void AssignComponent(entt::registry &ecs, entt::entity target) override
    {
        if(!ecs.valid(target))
        {
            // Entity died before we could set its component - tired of waiting? :(
            return;
        }
        ecs.assign_or_replace<TAsset>(target, std::move(asset));
    }
};

} // namespace boyd
