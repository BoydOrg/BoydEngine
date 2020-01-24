#pragma once

#include <fmt/color.h>
#include <fmt/format.h>
#include <iostream>
#include <utility>

#include "../Core/Platform.hh"
#include "BoydBuildConfig.hh"

namespace boyd
{

/// A logging level for `BOYD_LOG()`.
enum class LogLevel
{
    Debug,
    Info,
    Warn,
    Error,
    Crit,
    Min = Debug,
    Max = Crit,
};

/// Global (singleton) logging system.
class BOYD_API Log
{
private:
    Log() = default;

    Log(const Log &toCopy) = delete;
    Log &operator=(const Log &toCopy) = delete;
    Log(Log &&toMove) = delete;
    Log &operator=(Log &&toMove) = delete;

public:
    ~Log() = default;

    static Log &instance()
    {
        static Log inst;
        return inst;
    }

    /// -- Do not use this directly - use the `BOYD_LOG()` macro! --
    /// Logs a message in fmtlib format as it came from a certain file:line pair.
    template <typename... Args>
    void log(LogLevel level, const char *file, unsigned line, fmt::basic_string_view<char> fmt, Args &&... fmtArgs)
    {
        static const char LEVEL_NAMES[] = {'D', 'I', 'W', 'E', 'C'};

        // TODO: Make the buffer thread-local?
        static thread_local fmt::memory_buffer buffer;
        buffer.clear();
        fmt::format_to(buffer, fmt, std::forward<Args>(fmtArgs)...);

        // TODO: Ability to register/deregister log streams as needed
        char levelChar = LEVEL_NAMES[unsigned(level) - unsigned(LogLevel::Min)];
        std::clog << file << ':' << line << " [" << levelChar << "] " << buffer.data() << std::endl;
    }
};

/// Log a message to the default log stream.
/// `level` is a `boyd::LogLevel`; the rest are format arguments in fmtlib "Python" format.
#ifndef BOYD_CXX_MSVC
//  Use the ##__VA_ARGS__ GCC extension
#    define BOYD_LOG(level, fmt, ...) \
        boyd::Log::instance().log(boyd::LogLevel::level, &__FILE__[BoydEngine__FILE__OFFSET], __LINE__, FMT_STRING(fmt), ##__VA_ARGS__)
#else
//  MSVC automatically suppresses the __VA_ARGS__ trailing comma
#    define BOYD_LOG(level, fmt, ...) \
        boyd::Log::instance().log(boyd::LogLevel::level, &__FILE__[BoydEngine__FILE__OFFSET], __LINE__, FMT_STRING(fmt), __VA_ARGS__)
#endif

} // namespace boyd