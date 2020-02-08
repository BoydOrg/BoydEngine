#pragma once

#include "../Core/Platform.hh"
#include "../Debug/Log.hh"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

#ifdef BOYD_PLATFORM_WIN32
#    import <windows.h>
#    define dlopen(library, flag) ((void *)LoadLibrary(name))
#    define dlclose(handle) FreeLibrary((HMODULE)handle)
#    define dlsym(handle, sym) (void *)GetProcAddress((HMODULE)handle, sym)
#else
#    include <dlfcn.h>
#endif

namespace boyd
{

/// Just a wrapper to a module.
/// Avoid using this directly!
struct BoydModule
{
    std::string modname;
    void *(*InitFunc)(void){nullptr};
    void (*UpdateFunc)(void *){nullptr};
    void (*HaltFunc)(void *){nullptr};

    ~BoydModule()
    {
        if(HaltFunc)
        {
            HaltFunc(data);
        }
        InitFunc = nullptr;
        UpdateFunc = nullptr;
        HaltFunc = nullptr;
    }
    void *data{nullptr};
    int priority{0};

    void Update()
    {
        if(UpdateFunc)
        {
            UpdateFunc(data);
        }
    }

    /// Orders two modules by their priority in ascending order
    inline bool operator<(const BoydModule &other) const
    {
        return priority < other.priority;
    }
};

#ifdef BOYD_PLATFORM_POSIX
constexpr int _prefixSkip = 3;
#else
constexpr int _prefixSkip = 0;
#endif

/// A wrapper over a shared library
class Dll : public BoydModule
{
    void *handle{nullptr};

    std::string initFuncName;
    std::string updateFuncName;
    std::string haltFuncName;

    template <typename FuncType>
    void CheckSymbol(const std::string &symbolName, FuncType &function)
    {
        if(!(function = reinterpret_cast<FuncType>(dlsym(handle, symbolName.c_str()))))
            BOYD_LOG(Error, "{}", dlerror());
    }

    void ReloadSymbols()
    {
        CheckSymbol(initFuncName, InitFunc);
        CheckSymbol(updateFuncName, UpdateFunc);
        CheckSymbol(haltFuncName, HaltFunc);
    }

    void Free()
    {
        // Need to destroy the module instance before unloading the function pointers...
        this->BoydModule::~BoydModule();
        if(handle)
        {
            dlclose(handle);
            handle = nullptr;
        }
        // The parent constructor will be called.
    }

public:
    /// Acquire a library and call its init method.
    /// The modules are expected to export three symbols:
    /// - `void BoydInit_<libname>(void)`,
    /// - `void* BoydUpdate_<libname>(void* data)`
    /// - `void* BoydHalt_<libname>(void* data)`
    /// where `<libname>` is the name of the library without the "lib" prefix
    /// and the trailing extension.
    ///
    /// Note: this wrapper strictly forbids copy constructors.
    const std::filesystem::path filepath;
    Dll(std::string modname, int priority)
        : filepath{GetModulePath(modname)}
    {
        BOYD_LOG(Info, "Loading module {}", modname);

        this->modname = modname;
        this->priority = priority;

        BOYD_LOG(Debug, "Trying to access {}", filepath.string());

        initFuncName = std::string{"BoydInit_"} + modname;
        updateFuncName = std::string{"BoydUpdate_"} + modname;
        haltFuncName = std::string{"BoydHalt_"} + modname;

        Reload();
    }

    // Avoid spurious problems when sharing libraries.
    Dll(const Dll &toCopy) = delete;
    Dll &operator=(const Dll &toCopy) = delete;

    Dll(Dll &&toMove)
    {
        *this = std::move(toMove);
    }

    Dll &operator=(Dll &&toMove)
    {
        this->handle = toMove.handle;
        this->data = toMove.data;
        this->InitFunc = toMove.InitFunc;
        this->initFuncName = toMove.initFuncName;
        this->UpdateFunc = toMove.UpdateFunc;
        this->updateFuncName = toMove.updateFuncName;
        this->HaltFunc = toMove.HaltFunc;
        this->haltFuncName = toMove.haltFuncName;

        // invalidate handle and function pointers
        toMove.handle = nullptr;
        toMove.data = nullptr;
        toMove.InitFunc = nullptr;
        toMove.UpdateFunc = nullptr;
        toMove.HaltFunc = nullptr;

        return *this;
    }

    ~Dll()
    {
        Free();
    }

    /// Reload the module by reloading all the exported names.
    /// Note that this method is not thread-safe! Plase avoid calling any
    /// symbol function during a `Reload()`.
    void Reload()
    {
        Free();

        handle = dlopen(filepath.c_str(), RTLD_NOW | RTLD_LOCAL);
        if(!handle)
        {
            BOYD_LOG(Error, "Failed to dlopen {}: {}", filepath.c_str(), dlerror());
            return;
        }
        ReloadSymbols();
        if(InitFunc)
        {
            data = InitFunc();
        }
    }

    /// Given a library path, extract the name of the module
    /// `path`: the path of the module. The extracted module will be the name of
    ///         the module without the extension and possibly the prefix
    static std::string GetModuleName(const std::filesystem::path &path)
    {
        std::filesystem::path filename = path.filename();
        return filename.stem().string().substr(_prefixSkip);
    }

    static std::filesystem::path GetModulePath(const std::string name)
    {
        return std::filesystem::path{"modules/"} +=
#ifdef BOYD_PLATFORM_POSIX
               ("lib" + name + ".so")
#else
               (name + ".dll")
#endif
            ;
    }
}; // namespace boyd

} // namespace boyd
