#include "../../Components/LoadRequest.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include "AllLoaders.hh"
#include "LoadedAsset.hh"
#include "Loader.hh"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace boyd
{

/// A request to load an asset.
struct JobToLoad
{
    entt::entity target;  ///< The entity to load the component for.
    std::string filepath; ///< Path of the file containing the asset.
    ENTT_ID_TYPE typeId;  ///< EnTT typeId of the component to be set in `target`.
};

/// An asset that was loaded
struct LoadedJob
{
    entt::entity target;                          ///< The entity to set the component to.
    std::unique_ptr<LoadedAssetBase> loadedAsset; ///< The asset that was loaded.

    LoadedJob(entt::entity target, decltype(loadedAsset) &&loadedAsset)
        : target{target}, loadedAsset{std::move(loadedAsset)}
    {
    }
};

class BOYD_API BoydAssetLoaderState
{
    std::thread workerThread;
    std::atomic<bool> running;

    // Input queue, to `workerThread`
    std::deque<JobToLoad> workToDo;
    std::mutex workToDoMutex;
    std::condition_variable workToDoCondVar;

    // Output queue, from `workerThread`
    std::deque<LoadedJob> loadedAssets;
    std::mutex loadedAssetsMutex;

public:
    LoaderMap loaders;
    entt::observer loadReqObserver;

    BoydAssetLoaderState(entt::registry &ecs)
    {
        RegisterAllLoaders(loaders);
        BOYD_LOG(Debug, "Asset loaders registered");

        running = true;
        workerThread = std::thread(&BoydAssetLoaderState::WorkerLoop, this);
        BOYD_LOG(Debug, "Asset loading thread started");

        loadReqObserver.connect(ecs, entt::collector.group<comp::LoadRequest>());
    }

    ~BoydAssetLoaderState()
    {
        loadReqObserver.disconnect();

        running = false;
        workToDoCondVar.notify_one();
        workerThread.join();
        BOYD_LOG(Debug, "Asset loading thread stopped");
    }

    /// Adds a new job for the loader thread to load.
    void AddJob(JobToLoad &&job)
    {
        {
            std::unique_lock<std::mutex> lock{workToDoMutex};
            workToDo.emplace_back(std::move(job));
        }
        workToDoCondVar.notify_one();
    }

    /// Attach all components that were loaded by to their respective entities in the `ECS`.
    /// Returns the number of components that were attached.
    size_t AttachLoadedAssets(entt::registry &ecs)
    {
        std::unique_lock<std::mutex> lock{loadedAssetsMutex};
        size_t nApplied = 0;
        for(; !loadedAssets.empty(); nApplied++)
        {
            auto &jobResult = loadedAssets.front();
            jobResult.loadedAsset->AssignComponent(ecs, jobResult.target);
            loadedAssets.pop_front();
        }
        return nApplied;
    }

private:
    void WorkerLoop()
    {
        while(running)
        {
            // Pop a job from the queue (waiting for one if there isn't any)...
            JobToLoad job;
            {
                std::unique_lock<std::mutex> lock{workToDoMutex};
                workToDoCondVar.wait(lock, [this]() {
                    return !running.load() || workToDo.empty();
                });
                if(!running)
                {
                    break;
                }
                job = workToDo.front();
                workToDo.pop_front();
            }

            // ...then execute it...
            auto it = loaders.find(job.typeId);
            if(it == loaders.end())
            {
                BOYD_LOG(Error, "Can't load {}: Loader for typeId={} not found!", job.filepath, job.typeId);
                continue;
            }
            auto loadedAsset = it->second(job.filepath);
            if(!loadedAsset)
            {
                BOYD_LOG(Error, "Error loading {} (component typeId={})", job.filepath, job.typeId);
                continue;
            }

            // ...and finally post the results to the main thread
            {
                std::unique_lock<std::mutex> lock{loadedAssetsMutex};
                loadedAssets.emplace_back(job.target, std::move(loadedAsset));
            }
        }
    }
};

} // namespace boyd

inline static boyd::BoydAssetLoaderState *GetState(void *state)
{
    return reinterpret_cast<boyd::BoydAssetLoaderState *>(state);
}

extern "C" {
BOYD_API void *BoydInit_AssetLoader()
{
    BOYD_LOG(Info, "Starting asset loader module");
    auto *gameState = Boyd_GameState();
    return new boyd::BoydAssetLoaderState(gameState->ecs);
}

BOYD_API void BoydUpdate_AssetLoader(void *statePtr)
{
    auto *gameState = Boyd_GameState();
    auto *state = GetState(statePtr);

    // Each time a load request component is added: enqueue a load request into the worker thread
    state->loadReqObserver.each([state, gameState](auto entity) {
        auto loadReq = gameState->ecs.get<boyd::comp::LoadRequest>(entity);
        state->AddJob({entity, loadReq.filepath, loadReq.componentType});
    });

    // Then, for each asset that was loaded, attach it to the right entity in the ECS
    state->AttachLoadedAssets(gameState->ecs);
}

BOYD_API void BoydHalt_AssetLoader(void *statePtr)
{
    BOYD_LOG(Info, "Halting asset loader module");
    auto *state = GetState(statePtr);
    delete GetState(state);
}
}
