#include "3rdparty.hh"
#include "../../Core/Registrar.hh"
#include "../../Core/Utils.hh"
#include "../../Debug/Log.hh"

#include <fmt/format.h>
#include <functional>

#include <cctype>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>

using namespace glm;

namespace boyd
{

namespace Frame
{
glm::vec3 zero{0.0f, 0.0f, 0.0f};
glm::vec3 top{0.0f, 1.0f, 0.0f};
glm::vec3 right{1.0f, 0.0f, 0.0f};
glm::vec3 bottom{0.0f, -1.0f, 0.0f};
glm::vec3 left{-1.0f, 1.0f, 0.0f};
glm::vec3 front{0.0f, 0.0f, -1.0f};
glm::vec3 back{0.0f, 0.0f, 1.0f};
} // namespace Frame

/// A hack to get LuaBridge to work
template <typename T>
T GetZeroVector() { return T{}; }

/// Capitalize a string
std::string capitalize(std::string s)
{
    return std::string{(char)toupper(s[0])} + s.substr(1);
}

template <int N, glm::qualifier Q>
std::vector<float> ToVector(const glm::vec<N, float, Q> *vec)
{
    return {(float *)vec, ((float *)vec) + N};
}

void RegisterGLM(luabridge::Namespace &ns)
{
    ns = ns.beginNamespace("utils");

    // clang-format off

#define REGISTER_VEC(T, constructor)                                                                                               \
    ns = ns.beginClass<T>(capitalize(#T).c_str())                                                                                  \
             .addConstructor<void(*) constructor>()                                                                                \
             .addFunction("normalize", std::function<T(const T *)> ([](const T* v) {CheckNull(v); return normalize(*v);}))                       \
             .addFunction("__add", std::function<T(const T *, const T *)>([](const T *a, const T *b) { CheckNull(a, b); return *a + *b; }))         \
             .addFunction("__sub", std::function<T(const T *, const T *)>([](const T *a, const T *b) { CheckNull(a, b); return *a - *b; }))         \
             .addFunction("__unm", std::function<T(const T *)>([](const T *a) { CheckNull(a); return -*a; }))                                    \
             .addFunction("__mul", std::function<float(const T *, const T *)>([](const T *a, const T *b) { CheckNull(a, b); return dot(*a, *b); })) \
             .addFunction("__mul", std::function<T(const T *, float k)>([](const T *v, float k) { CheckNull(v); return k * (*v); }))             \
             .addFunction("__tostring", std::function<std::string(const T *)>([](const T *a) { CheckNull(a); return glm::to_string(*a); }))      \
             .addStaticProperty("zero", GetZeroVector<T>)                                                                          \
        .endClass()
    // clang-format on
    //.addFunction("as_vector", ToVector)

    REGISTER_VEC(vec2, (float, float));
    REGISTER_VEC(vec3, (float, float, float));
    REGISTER_VEC(vec4, (float, float, float, float));

    // clang-format off
    ns = ns.beginClass<vec2>("Vec2")
             .addProperty("x", &vec2::x, true)
             .addProperty("y", &vec2::y, true)
        .endClass()
        .beginClass<vec3>("Vec3")
             .addFunction("cross", std::function<vec3(const vec3 *, const vec3 *)>([](const vec3 *a, const vec3 *b) { return cross(*a, *b); }))
             .addProperty("x", &vec3::x, true)
             .addProperty("y", &vec3::y, true)
             .addProperty("z", &vec3::z, true)
        .endClass()
        .beginClass<vec4>("Vec4")
             .addProperty("x", &vec4::x, true)
             .addProperty("y", &vec4::y, true)
             .addProperty("z", &vec4::z, true)
             .addProperty("w", &vec4::w, true)
        .endClass();

    // clang-format on

    ns = ns.endNamespace();

    // Some convenience functions for when defining translations.
    // The directions are meant to be relative to the local transform and not
    // absolute, even though the user can freely use them in world space
    // directly by using an Identity transform.
    // For example, if we wanted to move our character forward by two meters
    /// we could do:
    // ```lua
    // new_tranfs = transf:move_relatively(boyd.Frame.front * 2)
    // transfComp:set(new_tranfs)
    // ```
    // It should be noted, it is always a good idea to keep two transforms.

    // clang-format off
    ns = ns.beginNamespace("frame")
        .addProperty("zero", &Frame::zero)
        .addProperty("top", &Frame::top)
        .addProperty("bottom", &Frame::bottom)
        .addProperty("left", &Frame::left)
        .addProperty("right", &Frame::right)
        .addProperty("front", &Frame::front)
        .addProperty("back", &Frame::back)
    .endNamespace();
    // clang-format on
}
} // namespace boyd