#pragma once

#include "../Core/Platform.hh"
#include "Dll.hh"
#include <filesystem>
#include <string>
#include <vector>

namespace boyd
{

using std::string;
using std::filesystem::path;

void InsertionSortLast();

#ifdef BOYD_HOT_RELOADING
extern std::vector<Dll> modules;
///
/// Avoid calling this directly! Use
/// Register a module to load.
/// `moduleName` - the filename of the module without the lib/ prefix if any
/// `priorityNo` - the order of which the module should be executed
///                during an update
void RegisterModule(const string &moduleName, int priorityNo);

#    define BOYD_MODULE(name, priority) RegisterModule(#    name, priority);

#else

// In order to avoid object slicing, hairy pointers and/or cyclical
// dependencies, there is no `extern vector<Dll> modules` here. This
// declaration is here for use only in BOYD_MODULE macro.
extern std::vector<BoydModule> modules;

#    define BOYD_MODULE(name, priority)       \
        modules.push_back({#name,             \
                           BoydInit_##name,   \
                           BoydUpdate_##name, \
                           BoydHalt_##name,   \
                           BoydInit_##name(), \
                           priority});        \
                                              \
        InsertionSortLast();

#endif

/// Call the modules' `Update` method. The order of the modules was
/// given by the priority number in `RegisterModule`.
void UpdateModules();

#ifdef BOYD_HOT_RELOADING
/// Reload a module. Avoid doing this manually as the listener will already
/// do this for you
/// `moduleName` - the module to reload
void ReloadModule(const string &moduleName);

/// Set a listener. New modules will be reloaded from here
/// `moduleFolder`: the folder to listen to
/// `waitTime`: the amount of time (in millisecs) to wait between two reads.
void SetListener(const path &moduleFolder, int waitTime);

/// Close the listener.
void CloseListener();
#endif
} // namespace boyd
