#include "3rdparty.hh"
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>

using namespace glm;

namespace boyd
{
void RegisterGLM(luabridge::Namespace &ns)
{
    ns = ns.beginNamespace("input");

#define REGISTER_VEC(T, constructor)                                                                                               \
    ns = ns.beginClass<T>(#T)                                                                                                      \
             .addConstructor<void(*) constructor>()                                                                                \
             .addFunction("__index", std::function<float(const T *, int i)>([](const T *vec, int i) { return (*vec)[i]; }))        \
             .addFunction("__add", std::function<T(const T *, const T *)>([](const T *a, const T *b) { return *a + *b; }))         \
             .addFunction("__sub", std::function<T(const T *, const T *)>([](const T *a, const T *b) { return *a - *b; }))         \
             .addFunction("__unm", std::function<T(const T *)>([](const T *a) { return -*a; }))                                    \
             .addFunction("__mul", std::function<float(const T *, const T *)>([](const T *a, const T *b) { return dot(*a, *b); })) \
             .addFunction("__mul", std::function<T(const T *, float k)>([](const T *v, float k) { return k * (*v); }))             \
             .addFunction("__tostring", std::function<std::string(const T *)>([](const T *a) { return glm::to_string(*a); }))      \
             .endClass()

    REGISTER_VEC(vec2, (float, float));
    REGISTER_VEC(vec3, (float, float, float));
    REGISTER_VEC(vec4, (float, float, float, float));

    ns = ns.endNamespace();
}
} // namespace boyd