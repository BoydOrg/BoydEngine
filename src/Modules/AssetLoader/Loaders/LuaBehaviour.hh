#pragma once

#include "../../../Components/LuaBehaviour.hh"
#include "../../../Core/Utils.hh"

namespace boyd
{

template <>
struct Loader<comp::LuaBehaviour>
{
    static std::unique_ptr<LoadedAssetBase> Load(std::string filepath)
    {
        std::string buffer;
        if(Slurp(filepath, buffer))
        {
            return std::make_unique<LoadedAsset<comp::LuaBehaviour>>(std::move(buffer));
        }
        else
        {
            return nullptr;
        }
    }
};

} // namespace boyd
