#pragma once

#include "../Core/Platform.hh"
#include "../Debug/Log.hh"

#include <string>
#include <filesystem>
#include <cstdint>
#include <iostream>
#include <cstring>

#ifdef BOYD_PLATFORM_WIN32
#   import <windows.h>
#   define dlopen(library, flag) ((void*) LoadLibrary(name))
#   define dlclose(handle) FreeLibrary((HMODULE) handle)
#   define dlsym(handle, sym) (void*) GetProcAddress((HMODULE) handle, sym)
#else
#   include <dlfcn.h>
#endif

namespace boyd{

#ifdef BOYD_PLATFORM_POSIX
        constexpr int _prefixSkip = 3;
#else
        constexpr int _prefixSkip = 0;
#endif    

/// Given a library path, extract the name of the module
/// `path`: the path of the module. The extracted module will be the name of
///         the module without the extension and possibly the prefix
std::string GetModuleName(const std::filesystem::path& path)
{
    std::filesystem::path filename = path.filename();
    return filename.stem().string().substr(_prefixSkip);
}

/// A wrapper over a shared library
class Dll {
    std::string initFuncName;
    std::string updateFuncName;
    std::string haltFuncName;

    void* handle {nullptr};
    void* (*initFunc) (void) {nullptr};
    void  (*updateFunc) (void*) {nullptr};
    void  (*haltFunc) (void*) {nullptr};
    void *data;

    template<typename FuncType>
    void CheckSymbol(const string& symbolName, FuncType& function)
    {
        if(!(function = reinterpret_cast<FuncType>(dlsym(handle, symbolName.c_str()))))
            BOYD_LOG(Error, "{}", dlerror());
    }

    void ReloadSymbols()
    {
        CheckSymbol(initFuncName, initFunc);
        CheckSymbol(updateFuncName, updateFunc);
        CheckSymbol(haltFuncName, haltFunc);
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
    std::string modname;

    Dll(std::filesystem::path filepath)
        : filepath{filepath},
          modname{GetModuleName(filepath)}
    {
        BOYD_LOG(Info, "Loading module {}", modname);
        BOYD_LOG(Debug, "Trying to access {}", filepath.string());

        initFuncName     = std::string{"BoydInit_"} + modname;
        updateFuncName   = std::string{"BoydUpdate_"} + modname;
        haltFuncName     = std::string{"BoydHalt_"} + modname;

        Reload();
    }

    // Avoid spurious problems when sharing libraries.
    Dll(const Dll& toCopy) = delete;
    Dll& operator=(const Dll& toCopy) = delete;

    Dll(Dll&& toMove)
    {
        *this = std::move(toMove);
    }

    Dll& operator=(Dll&& toMove)
    {
        this->handle = toMove.handle;
        this->initFunc = toMove.initFunc;
        this->initFuncName = toMove.initFuncName;
        this->updateFunc = toMove.updateFunc;
        this->updateFuncName = toMove.updateFuncName;
        this->haltFunc = toMove.haltFunc;
        this->haltFuncName = toMove.haltFuncName;

        // invalidate handle and function pointers
        toMove.handle = nullptr;
        toMove.initFunc = nullptr;
        toMove.updateFunc = nullptr;
        toMove.haltFunc = nullptr;

        return *this;
    }

    ~Dll()
    {
        if(haltFunc)
        {
            haltFunc(data);
            data = nullptr;
        }
        if (handle)
        {
            dlclose(handle);
            handle = nullptr;
        }
        // invalidate handle and function pointers
        initFunc = nullptr;
        updateFunc = nullptr;
        haltFunc = nullptr;
    }
    
    /// Call the Update function inside the module.
    void Update()
    {
        updateFunc(data);
    }

    /// Reload the module by reloading all the exported names.
    /// Note that this method is not thread-safe! Plase avoid calling any
    /// symbol function during a `Reload()`.
    void Reload()
    {
        (void) this->~Dll();
        handle = dlopen(filepath.c_str(), RTLD_NOW | RTLD_LOCAL);
        ReloadSymbols();
        data = initFunc();
    }
};

}