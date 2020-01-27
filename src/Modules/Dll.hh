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

/// A wrapper over a shared library
class Dll {
    std::string initFuncName;
    std::string updateFuncName;
    std::string haltFuncName;

    void* handle;
    void* (*initFunc) (void);
    void  (*updateFunc) (void*);
    void  (*haltFunc) (void*);
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
    const std::string filename;
    std::string modname;

    Dll(std::filesystem::path filepath)
        : filepath{filepath},
          filename{filepath.filename()}
    {
        // Take the filename only, strip the "lib" prefix and remove the extension
        modname = filename.substr(3, filename.size() - 3 - filepath.extension().string().size());

        initFuncName     = std::string{"BoydInit_"} + modname;
        updateFuncName   = std::string{"BoydUpdate_"} + modname;
        haltFuncName     = std::string{"BoydHalt_"} + modname;

        ReloadSymbols();
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
        haltFunc(data);
        dlclose(handle);
    }
    
    /// Call the Update function inside the module
    void Update()
    {
        updateFunc(data);
    }

    /// Reload the module by reloading all the exported names
    void Reload()
    {
        (void) this->~Dll();
        handle = dlopen(filepath.c_str(), RTLD_NOW | RTLD_LOCAL);
        ReloadSymbols();
        data = initFunc();
    }
};

}