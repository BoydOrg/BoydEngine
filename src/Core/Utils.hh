#pragma once

#include <fstream>
#include <string>

namespace boyd
{

/// Reads a whole file to `buffer`. Returns false on error.
inline bool Slurp(std::string filepath, std::string &buffer)
{
    std::ifstream infile{filepath};
    if(!infile)
    {
        return false;
    }

    // https://stackoverflow.com/a/2602060 - should be the fastest way to do it
    infile.seekg(0, infile.end);
    buffer.reserve(infile.tellg());
    infile.seekg(0, infile.beg);
    buffer.assign(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>());

    return true;
}

} // namespace boyd