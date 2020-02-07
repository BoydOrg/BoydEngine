#pragma once

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

#include "LoadedAsset.hh"

namespace boyd
{

/// A function that loads a `TAsset` from filepath (returns nullptr on error).
using LoaderFunc = std::unique_ptr<LoadedAssetBase> (*)(std::string filepath);

/// A map of <TAsset type id -> asset LoaderFunc for that TAsset type>
using LoaderMap = std::unordered_map<ENTT_ID_TYPE, LoaderFunc>;

/// Loads assets of type TAsset from file.
template <typename TAsset>
struct Loader
{
    /// Loads a `TAsset` from the given filepath. Returns null on loading error.
    static std::unique_ptr<LoadedAssetBase> Load(std::string filepath);
};

} // namespace boyd
