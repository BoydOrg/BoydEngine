#include "Loader.hh"
#include "Dll.hh"

#include <thread>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <unordered_map>
#include <mutex>

#ifdef BOYD_PLATFORM_POSIX
#   include <sys/time.h>
#   include <sys/inotify.h>
#   include <unistd.h>
#endif

namespace boyd {

using std::string;
using std::vector;
using std::pair;
using std::filesystem::path;

using BoydPriorityModule = pair<int, Dll>;

static vector<BoydPriorityModule> modules;

static std::thread* listener;
static bool quitPoller = false;
static std::mutex lockUpdates;

// Basic insertion sort to sort the modules internally
static void InsertionSortLast()
{
    for (auto curElem = modules.end() - 1; curElem != modules.begin(); curElem--)
        if (curElem->first > (curElem - 1)->first)
            std::swap(*curElem, *(curElem - 1));
}

void RegisterModule(const string& moduleName, int priorityNo)
{
#ifdef BOYD_PLATFORM_POSIX
    path modulePath = string{"lib/"} + moduleName;
#else
    path modulePath{moduleName};
#endif
    modules.emplace_back(priorityNo, modulePath);
    InsertionSortLast();
}

void UpdateModules()
{
    // Avoid updating in case of a reload
    std::unique_lock<std::mutex> lockGuard(lockUpdates);
    for(auto& module: modules)
        module.second.Update();
}

void ReloadModule(const string& moduleName)
{
    std::unique_lock<std::mutex> lockGuard(lockUpdates);
    // Find the first module that matches the name
    auto it = std::find_if(modules.begin(), modules.end(),
        [&moduleName](const BoydPriorityModule& a)
        {
            return a.second.modname == moduleName;
        });

    if (it == modules.end())
    {
        BOYD_LOG(Warn, "The module {} was not registered - not reloading it", moduleName);
    }

    // Recursively reload the dependencies
    else
    {
        it->second.Reload();
    }
}


#ifdef BOYD_PLATFORM_POSIX
static void EventPoller(int inotifyFd, int waitFor)
{
    constexpr size_t EVENT_SIZE = sizeof (struct inotify_event);
    constexpr size_t MAX_BUF_LEN = 1024 * (EVENT_SIZE + 16);

    struct timeval interval;
    fd_set rfds;
    int ret;

    int eventBufferSize;
    char eventBuffer[MAX_BUF_LEN];

    interval.tv_usec = waitFor;

    while(!quitPoller)
    {
        FD_ZERO(&rfds);
        FD_SET(inotifyFd, &rfds);

        // Check for new events. If after `waitTime` there are none wait again
        ret = select (inotifyFd+1, &rfds, nullptr, nullptr, &interval);
        if (ret < 0)
        {
            BOYD_LOG(Warn, "Could not enable use select(): {}", strerror(errno));
        } else if (!ret)
            // check next time
            continue;
        else if (FD_ISSET(inotifyFd, &rfds))
        {
            int bufferCursor = 0;
            eventBufferSize = read(inotifyFd, eventBuffer, MAX_BUF_LEN);

            if (eventBufferSize < 0)
            {
                BOYD_LOG(Warn, "Could not read(): {}", strerror(errno));
                continue;
            }
            while (bufferCursor < eventBufferSize)
            {
                inotify_event* event = (inotify_event*)
                                            &eventBuffer[bufferCursor];
                if (event->len)
                {
                    path modifiedModule = event->name;
                    BOYD_LOG(Info, "{} has changed, reloading...", modifiedModule.string());
                    ReloadModule(GetModuleName(modifiedModule));
                }
                bufferCursor += EVENT_SIZE + event->len;
            }
        }
    }
}
#endif

void SetListener(const path& modulePath, int waitFor)
{
#ifdef BOYD_PLATFORM_POSIX
    int inotify_fd, inotify_watcher;
    inotify_fd = inotify_init();
    if (inotify_fd < 0)
    {
        BOYD_LOG(Warn, "Error while setting up inotify: {}", strerror(errno));
    }
    else
    {
        inotify_watcher = inotify_add_watch(inotify_fd, modulePath.c_str(), IN_CLOSE_WRITE);
        if (inotify_watcher < 0)
        {
            BOYD_LOG(Warn, "Error while setting up inotify: {}", strerror(errno));
        }
        else
        {
            quitPoller = false;
            listener = new std::thread(EventPoller, inotify_fd, 1000 * waitFor);
        }
    }
#else
    BOYD_LOG(Warn, "Hot reloading and listeners are not yet supported on windows");
#endif
}

void CloseListener()
{
    quitPoller = true;
    listener->join();
    delete listener;
    listener = nullptr;
}
} // namespace boyd