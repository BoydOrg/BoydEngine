#include "../../Components/ComponentLoadRequest.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include "LoadedAsset.hh"
#include "Loader.hh"
#include "Loaders/AllLoaders.hh"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace boyd
{

/// A request to load an asset.
struct LoadJob
{
    entt::entity target;  ///< The entity to load the component for.
    ENTT_ID_TYPE typeId;  ///< EnTT typeId of the component to be set in `target`.
    std::string filepath; ///< Path of the file containing the asset.

    LoadJob()
        : target{entt::null}, typeId{}, filepath{}
    {
    }
    LoadJob(entt::entity target, ENTT_ID_TYPE typeId, std::string filepath)
        : target{target}, typeId{typeId}, filepath{filepath}
    {
    }
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
    std::deque<LoadJob> jobs;
    std::atomic<bool> hasJobs; // NOTE: To be able to know if `jobs.empty()` without locking `jobsMutex`!
    std::mutex jobsMutex;
    std::condition_variable jobsCondVar;

    // Output queue, from `workerThread`
    std::deque<LoadedJob> loadedAssets;
    std::mutex loadedAssetsMutex;

public:
    LoaderMap loaders;
    entt::observer loadReqObserver;

    BoydAssetLoaderState(entt::registry &ecs)
        : jobs{}, loadedAssets{}, loadReqObserver{ecs, entt::collector.group<comp::ComponentLoadRequest>()}
    {
        RegisterAllLoaders(loaders);
        BOYD_LOG(Debug, "Asset loaders registered");

        hasJobs = false;
        running = true;
        workerThread = std::thread(&BoydAssetLoaderState::WorkerLoop, this);
        BOYD_LOG(Debug, "Asset loading thread started");
    }

    ~BoydAssetLoaderState()
    {
        loadReqObserver.disconnect();

        hasJobs = true; // Fake having a new job to awake the worker thread from its `wait()`
        running = false;
        jobsCondVar.notify_one();
        workerThread.join();
        BOYD_LOG(Debug, "Asset loading thread stopped");
    }

    /// Adds new jobs for the loader thread to load depending on a `LoadRequest`.
    void AddJobs(entt::entity target, const comp::ComponentLoadRequest &loadReq)
    {
        {
            std::unique_lock<std::mutex> lock{jobsMutex};
            for(auto &it : loadReq.requests)
            {
                jobs.emplace_back(target, it.first, it.second);
            }
            hasJobs = true;
        }
        jobsCondVar.notify_one();
    }

    /// Attach all components that were loaded by to their respective entities in the `ECS`.
    /// Also detachs the LoadRequest on the component!
    /// Returns the number of components that were attached.
    size_t AttachLoadedAssets(entt::registry &ecs)
    {
        using CompReq = comp::ComponentLoadRequest;

        std::unique_lock<std::mutex> lock{loadedAssetsMutex};
        size_t nApplied = 0;
        for(; !loadedAssets.empty(); nApplied++)
        {
            auto &jobResult = loadedAssets.front();
            if(ecs.valid(jobResult.target))
            {
                jobResult.loadedAsset->AssignComponent(ecs, jobResult.target);
            }
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
            LoadJob job;
            {
                std::unique_lock<std::mutex> lock{jobsMutex};
                jobsCondVar.wait(lock, [this]() {
                    return hasJobs.load();
                });
                if(!running)
                {
                    break;
                }
                job = jobs.front();
                jobs.pop_front();
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
            BOYD_LOG(Debug, "Loaded {} for entity={}, typeId={}", job.filepath, job.target, job.typeId);

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

    // Each time a load request component is added:
    // - Enqueue a load request into the worker thread
    // - Remove the load request from the requester
    state->loadReqObserver.each([state, gameState](auto entity) {
        const auto &loadReq = gameState->ecs.get<boyd::comp::ComponentLoadRequest>(entity);
        state->AddJobs(entity, loadReq);
        gameState->ecs.remove<boyd::comp::ComponentLoadRequest>(entity);
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
