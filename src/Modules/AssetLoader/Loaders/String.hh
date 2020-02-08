#pragma once

#include "../../../Components/String.hh"
#include "../../../Core/Utils.hh"

namespace boyd
{

template <>
struct Loader<comp::String>
{
    static std::unique_ptr<LoadedAssetBase> Load(std::string filepath)
    {
        std::string buffer;
        if(Slurp(filepath, buffer))
        {
            return std::make_unique<LoadedAsset<comp::String>>(std::move(buffer));
        }
        else
        {
            return nullptr;
        }
    }
};

} // namespace boyd
