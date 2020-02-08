#include "AllLoaders.hh"

#include "../../Components/ComponentLoadRequest.hh"

// TODO: Automate this header list from ALL_LOADERS?
#include "AudioClip.hh"
#include "Gltf.hh"
#include "String.hh"

namespace boyd
{

void RegisterAllLoaders(LoaderMap &map)
{
#define BOYD_LOADER(component) \
    map[comp::ComponentLoadRequest::TypeOf<component>()] = &Loader<component>::Load;

    BOYD_ALL_LOADERS()
}

} // namespace boyd
