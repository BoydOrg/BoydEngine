#pragma once

#include "Loader.hh"

namespace boyd
{

/// Registers all known `TypeOf(TAsset) -> Loader<TAsset>` pairs to the given map.
void RegisterAllLoaders(LoaderMap &map);

} // namespace boyd
