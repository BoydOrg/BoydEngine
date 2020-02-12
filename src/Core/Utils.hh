#pragma once

#include <fmt/format.h>
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

// Utility to check if one or more parameters are not null
struct NilException : virtual public std::exception
{
    std::string error;
    NilException(std::string what)
        : error{what} {}

    virtual const char *what() const throw()
    {
        return error.c_str();
    }
};

template <int N>
int CheckParams() { return N; }

template <int N, typename GLM, typename... Args>
int CheckParams(const GLM *param, Args... params)
{
    if(!param)
        return N;
    return CheckParams<N + 1>(params...);
}

// Throw an exception when one parameter is null
template <typename... GLM>
void CheckNull(const GLM *... args)
{
    int N = CheckParams<0>(args...);
    if(N < sizeof...(args))
    {
        throw NilException(fmt::format("Parameter #{} was null", N + 1));
    }
}

} // namespace boyd