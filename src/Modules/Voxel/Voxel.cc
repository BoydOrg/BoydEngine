#include "../../Components/Voxels.hh"
#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
using namespace boyd;

struct BoydVoxelState
{
    entt::observer observer;

    BoydVoxelState()
    {
        auto *gameState = Boyd_GameState();
        // Observe every time an entity has both a Voxels and a VoxelsDirty component
        // (= when its voxels need to be remeshed)
        observer.connect(gameState->ecs, entt::collector.group<comp::Voxels, comp::VoxelsDirty>());
    }
    ~BoydVoxelState() = default;
};

inline static BoydVoxelState *GetState(void *statePtr)
{
    return reinterpret_cast<BoydVoxelState *>(statePtr);
}

extern "C" {

BOYD_API void *BoydInit_Voxel()
{
    BOYD_LOG(Info, "Starting Voxel module");
    return new BoydVoxelState();
}

BOYD_API void BoydUpdate_Voxel(void *state)
{
    auto *gameState = Boyd_GameState();
    GetState(state)->observer.each([gameState](entt::entity entity) {
        // TODO: Remesh voxels if voxel component changes!
    });
}

BOYD_API void BoydHalt_Voxel(void *state)
{
    BOYD_LOG(Info, "Halting Voxel module");
    delete GetState(state);
}
}
