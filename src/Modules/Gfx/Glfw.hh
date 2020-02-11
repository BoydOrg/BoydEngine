#pragma once

#include "../../Core/Platform.hh"

// NOTE: Must include flextGL (or the other GL header/loaders) *before* GLFW!
#if defined(BOYD_PLATFORM_WIN32)
//   Windows: Use ANGLE to get GLES3
#    define BOYD_GLES3_ANGLE
//   FIXME IMPLEMENT: Include ANGLE headers!
#elif defined(BOYD_PLATFORM_EMSCRIPTEN)
//   Emscripten: Use builtin headers
#    define BOYD_GLES3_EMSCRIPTEN
#    include <GLES3/gl3.h>
#    include <EGL/egl.h>
#else
//   Linux/other: Rely on flextGL and native GLES3 (e.g. Mesa)
#    define BOYD_GLES3_FLEXTGL
#    include <flextGL.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if defined(EMSCRIPTEN)
// Workaround for missing function
inline int glfwGetError(const char **outDescription)
{
    static constexpr const char DESCR[] = "<placeholder>";
    *outDescription = DESCR;
    return sizeof(DESCR);
}
#endif
