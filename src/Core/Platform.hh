#pragma once

#if defined(WIN32) || defined(_WIN32)
#    define BOYD_PLATFORM_WIN32
#elif defined(__EMSCRIPTEN__)
#    define BOYD_PLATFORM_EMSCRIPTEN
#else
#    define BOYD_PLATFORM_POSIX
#endif

#if defined(_MSC_VER)
#    define BOYD_CXX_MSVC
#elif defined(__GNUC__) || defined(__clang__)
#    define BOYD_CXX_GCCLIKE
#endif

#ifdef BOYD_CXX_MSVC
#    ifdef BOYD_HOT_RELOADING
#        define BOYD_API
#        ifdef BOYD_DLL_EXPORTS
#            define BOYD_API __declspec(dllexport)
#        else
#            define BOYD_API __declspec(dllimport)
#        endif
#    else
//       No need to export symbols if linking statically
#        define BOYD_API
#    endif
#elif defined(BOYD_CXX_GCCLIKE)
#    define BOYD_API __attribute__((visibility("default")))
#else
#    error "Unknown compiler!"
#endif

#if (defined(__i386__) || defined(__x86_64__))
#    if defined(BOYD_CXX_GCCLIKE)
#        define BOYD_DEBUGGER_TRAP() __asm__ __volatile__("int3")
#    elif defined(BOYD_PLATFORM_WIN32)
//       #include <Windows.h> -> #include <debugapi.h>
extern "C" {
__declspec(dllimport) void __stdcall DebugBreak();
}
#        define BOYD_DEBUGGER_TRAP() DebugBreak()
#   else
#        define BOYD_DEBUGGER_TRAP()
#   endif
#elif defined(BOYD_PLATFORM_EMSCRIPTEN)
#    include <emscripten.h>
#    define BOYD_DEBUGGER_TRAP() EM_ASM(debugger;)
#else
#    define BOYD_DEBUGGER_TRAP()
#endif
