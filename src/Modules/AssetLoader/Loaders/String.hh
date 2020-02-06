#pragma once

#include "../../../Components/String.hh"
#include <fstream>

namespace boyd
{

template <>
struct Loader<comp::String>
{
    static std::unique_ptr<LoadedAssetBase> Load(std::string filepath)
    {
        std::ifstream infile{filepath};
        if(!infile)
        {
            return nullptr;
        }

        // https://stackoverflow.com/a/2602060 - should be the fastest way to do it
        infile.seekg(0, infile.end);
        std::string buffer;
        buffer.reserve(infile.tellg());
        infile.seekg(0, infile.beg);
        buffer.assign(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>());

        return std::make_unique<LoadedAsset<comp::String>>(std::move(buffer));
    }
};

} // namespace boyd
