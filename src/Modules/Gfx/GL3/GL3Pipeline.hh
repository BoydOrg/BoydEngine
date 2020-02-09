#pragma once

#include "GL3.hh"

namespace boyd
{
namespace gl3
{

/// The OpenGL rendering pipeline.
struct Pipeline
{
    enum Stages : unsigned
    {
        Forward = 0, ///< Forward rendering + lighting.
        _Last = Forward,
    };
    struct Stage
    {
        gl3::SharedProgram program;
    };
    Stage stages[_Last + 1];

    /// Load all pipeline stages & their resources.
    Pipeline();
    ~Pipeline();

    inline Stage *begin()
    {
        return stages;
    }
    inline Stage *end()
    {
        return stages + _Last + 1;
    }
};

} // namespace gl3
} // namespace boyd
