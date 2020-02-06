#include "AllLoaders.hh"

#include "../../Components/ComponentLoadRequest.hh"

namespace boyd
{

// FIXME - TEST - Actually implement some real loaders!
template <>
struct Loader<std::string>
{
    static std::unique_ptr<LoadedAssetBase> Load(std::string filepath)
    {
        return std::make_unique<LoadedAsset<std::string>>("Hello, world!");
    }
};

void RegisterAllLoaders(LoaderMap &map)
{
    map[comp::ComponentLoadRequest::TypeOf<std::string>()] = &Loader<std::string>::Load;
}

} // namespace boyd
