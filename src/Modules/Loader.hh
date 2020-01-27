#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace boyd {

using std::string;
using std::filesystem::path;

/// Register a module to load.
/// `moduleName` - the filename of the module without the lib/ prefix if any
/// `priorityNo` - the order of which the module should be executed
///                during an update
void RegisterModule(const string& moduleName, int priorityNo);

/// Call the modules' `Update` method. The order of the modules was
/// given by the priority number in `RegisterModule`.
void UpdateModules();

/// Reload a module. Avoid doing this manually as the listener will already
/// do this for you
/// `moduleName` - the module to reload
void ReloadModule(const string& moduleName);

/// Set a listener. New modules will be reloaded from here
/// `moduleFolder`: the folder to listen to
/// `waitTime`: the amount of time (in millisecs) to wait between two reads.
void SetListener(const path& moduleFolder, int waitTime);

/// Close the listener.
void CloseListener();
}