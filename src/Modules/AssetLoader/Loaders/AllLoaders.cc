#include "AllLoaders.hh"

#include "../../Components/ComponentLoadRequest.hh"

// TODO: Automate this header list from ALL_LOADERS?
#include "String.hh"

namespace boyd
{

void RegisterAllLoaders(LoaderMap &map)
{
    map[comp::ComponentLoadRequest::TypeOf<comp::String>()] = &Loader<comp::String>::Load;
}

} // namespace boyd
