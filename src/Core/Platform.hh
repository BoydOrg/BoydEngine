#pragma once

#if defined(WIN32) || defined(_WIN32)
#    define BOYD_PLATFORM_WIN32
#else
#    define BOYD_PLATFORM_POSIX
#endif

#if defined(_MSC_VER)
#    define BOYD_CXX_MSVC
#elif defined(__GNUC__) || defined(__clang__)
#    define BOYD_CXX_GCCLIKE
#endif

#ifdef BOYD_CXX_MSVC
#    ifdef BOYD_DLL_EXPORTS
#        define BOYD_API __declspec(dllexport)
#    else
#        define BOYD_API __declspec(dllimport)
#    endif
#elif defined(BOYD_CXX_GCCLIKE)
#    define BOYD_API __attribute__((visibility("default")))
#else
#    error "Unknown compiler!"
#endif

#if defined(BOYD_CXX_GCCLIKE) && (defined(__i386__) || defined(__x86_64__))
#    define BOYD_DEBUGGER_TRAP() __asm__ __volatile__("int3")
#else
#    include <debugapi.h>
#    define BOYD_DEBUGGER_TRAP() DebugBreak()
#endif