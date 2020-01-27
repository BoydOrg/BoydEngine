#pragma once

#include <string>
#include <vector>

namespace boyd {

using std::string;

/// Register a module to load.
/// `moduleName` - the filename of the module without the lib/ prefix if any
/// `priorityNo` - the order of which the module should be executed
///                during an update
/// `moduleDeps` - specify a list of dependencies the module depends on.
///                In case a dependency is rebuilt then the module is
///                reloaded as well. The caller should enforce the
///                dependencies not to constitute cycles.
void RegisterModule(const string& moduleName, int priorityNo, const std::vector<string>& moduleDeps);

/// Call the modules' `Update` method. The order of the modules was
/// given by the priority number in `RegisterModule`.
void UpdateModules();

/// Reload a module. Avoid doing this manually as the listener will already
/// do this for you
/// `moduleName` - the module to reload
void ReloadModule(const string& moduleName);

/// Set a listener. New modules will be reloaded from here
/// `moduleFolder`: the folder to listen to
/// `waitTime`: the amount of time (in microseconds) to wait between two reads.
void SetListener(const string& moduleFolder, int waitTime);

/// Close the listener.
void CloseListener();
}