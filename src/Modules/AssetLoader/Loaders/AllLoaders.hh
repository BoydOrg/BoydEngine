#pragma once

#include "../Loader.hh"

namespace boyd
{

/// The list of all registered loadable types, one BOYD_LOADER(TComp) per line.
/// This registers each boyd::Loader<TComp> template, as defined in "AssetLoader/Loader.hh".
#define BOYD_ALL_LOADERS() \
    BOYD_LOADER(boyd::comp::String)

/// Registers all known `TypeOf(TAsset) -> Loader<TAsset>` pairs to the given map.
void RegisterAllLoaders(LoaderMap &map);

} // namespace boyd