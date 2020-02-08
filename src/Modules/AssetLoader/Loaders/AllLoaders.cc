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
    map[comp::ComponentLoadRequest::TypeOf<comp::String>()] = &Loader<comp::String>::Load;
    map[comp::ComponentLoadRequest::TypeOf<comp::AudioClip>()] = &Loader<comp::AudioClip>::Load;
    map[comp::ComponentLoadRequest::TypeOf<comp::Gltf>()] = &Loader<comp::Gltf>::Load;
}

} // namespace boyd
