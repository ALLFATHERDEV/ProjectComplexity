#pragma once

#include "../camera/Camera2D.hpp"
#include "../entities/ComponentStorage.hpp"
#include "../entities/EntityManager.hpp"
#include "../entities/component/FluidPipeComponent.hpp"
#include "../entities/component/FluidPumpComponent.hpp"
#include "../entities/component/FluidPortComponent.hpp"
#include "../entities/component/FluidTankComponent.hpp"
#include "../entities/component/CollisionComponent.hpp"
#include "../entities/component/ConveyorBeltComponent.hpp"
#include "../entities/component/CraftingMachineComponent.hpp"
#include "../entities/component/InputComponent.hpp"
#include "../entities/component/InteractionComponent.hpp"
#include "../entities/component/InventoryComponent.hpp"
#include "../entities/component/MachineComponent.hpp"
#include "../entities/component/MachineFluidComponent.hpp"
#include "../entities/component/MachineFluidPortLinkComponent.hpp"
#include "../entities/component/MachineInventoryComponent.hpp"
#include "../entities/component/MinerComponent.hpp"
#include "../entities/component/PositionComponent.hpp"
#include "../entities/component/SpriteComponent.hpp"
#include "../entities/component/VelocityComponent.hpp"
#include "../entities/component/AnimationControllerComponent.hpp"
#include "../entities/systems/FluidSystem.hpp"
#include "../fluid/FluidDefinition.hpp"
#include "../graphics/Renderer.hpp"
#include "AnimationLibrary.hpp"
#include "../graphics/SpriteAtlas.hpp"

#include <unordered_map>

class ChunkManager;

struct AtlasCoords {
    int x;
    int y;
};

struct PipeSpriteLayout {
    AtlasCoords isolatedVertical{0, 0};
    AtlasCoords isolatedHorizontal{1, 0};
    AtlasCoords endUp{8, 0};
    AtlasCoords endRight{9, 0};
    AtlasCoords endDown{6, 0};
    AtlasCoords endLeft{7, 0};
    AtlasCoords vertical{0, 0};
    AtlasCoords horizontal{1, 0};
    AtlasCoords cornerUpRight{3, 0};
    AtlasCoords cornerRightDown{4, 0};
    AtlasCoords cornerDownLeft{5, 0};
    AtlasCoords cornerLeftUp{2, 0};
    AtlasCoords teeNoLeft{12, 0};
    AtlasCoords teeNoUp{13, 0};
    AtlasCoords teeNoRight{14, 0};
    AtlasCoords teeNoDown{11, 0};
    AtlasCoords cross{10, 0};
    AtlasCoords tank{6, 1};
    AtlasCoords pumpUp{6, 3};
    AtlasCoords pumpRight{7, 3};
    AtlasCoords pumpDown{8, 3};
    AtlasCoords pumpLeft{9, 3};
};

class FluidManager {
public:
    FluidManager(EntityManager& entityManager,
                 ComponentStorage<PositionComponent>& positions,
                 ComponentStorage<FluidPipeComponent>& fluidPipes,
                 ComponentStorage<FluidTankComponent>& fluidTanks,
                 ComponentStorage<FluidPumpComponent>& fluidPumps,
                 ComponentStorage<FluidPortComponent>& fluidPorts,
                 ComponentStorage<SpriteComponent>& sprites,
                 ComponentStorage<VelocityComponent>& velocities,
                 ComponentStorage<InputComponent>& inputs,
                 ComponentStorage<CharacterStateComponent>& characterStates,
                 ComponentStorage<AnimationControllerComponent>& animationControllers,
                 ComponentStorage<CollisionComponent>& collisions,
                 ComponentStorage<ConveyorBeltComponent>& conveyorBelts,
                 ComponentStorage<InventoryComponent>& inventories,
                 ComponentStorage<MachineFluidComponent>& machineFluids,
                 ComponentStorage<MachineFluidPortLinkComponent>& machineFluidPortLinks,
                 ComponentStorage<MachineComponent>& machines,
                 ComponentStorage<MachineInventoryComponent>& machineInventories,
                 ComponentStorage<CraftingMachineComponent>& craftingMachines,
                 ComponentStorage<MinerComponent>& miners,
                 ComponentStorage<InteractionComponent>& interactions,
                 AnimationLibrary& animationLibrary,
                 FluidSystem& fluidSystem);

    void placeFluidPipe(int tileX, int tileY, Direction direction);
    void removeFluidPipe(int tileX, int tileY);
    void placeFluidTank(int tileX, int tileY);
    void removeFluidTank(int tileX, int tileY);
    void placeFluidPump(int tileX, int tileY, Direction direction);
    void removeFluidPump(int tileX, int tileY);
    bool addDebugFluidToTank(int tileX, int tileY, float amount);
    void renderDebug(Renderer& renderer, const Camera2D& camera, const ChunkManager& chunkManager) const;
    void setAtlas(SpriteAtlas* atlas);
    void registerFluidTankEntity(int tileX, int tileY, Entity entity);
    void registerFluidPumpEntity(int tileX, int tileY, Entity entity);
    void unregisterFluidTankEntity(int tileX, int tileY);
    void unregisterFluidPumpEntity(int tileX, int tileY);
    void setDefaultFluidDefinition(const FluidDefinition* fluidDefinition);
    const FluidDefinition* getDebugFluidDefinition() const;

private:
    static long long makeTileKey(int tileX, int tileY);
    Entity getFluidPipeAtTile(int tileX, int tileY) const;
    Entity getFluidTankAtTile(int tileX, int tileY) const;
    Entity getFluidPumpAtTile(int tileX, int tileY) const;
    void refreshFluidSpriteAt(int tileX, int tileY);
    void refreshFluidSpritesAround(int tileX, int tileY);
    bool hasConnectableNeighbor(int tileX, int tileY, Direction direction) const;
    bool hasCompatiblePortAtTile(int tileX, int tileY, Direction direction) const;
    Sprite getPipeSprite(const FluidPipeComponent& pipe) const;
    Sprite getTankSprite() const;
    Sprite getPumpSprite(Direction direction) const;

    EntityManager& m_EntityManager;
    ComponentStorage<PositionComponent>& m_Positions;
    ComponentStorage<FluidPipeComponent>& m_FluidPipes;
    ComponentStorage<FluidTankComponent>& m_FluidTanks;
    ComponentStorage<FluidPumpComponent>& m_FluidPumps;
    ComponentStorage<FluidPortComponent>& m_FluidPorts;
    ComponentStorage<SpriteComponent>& m_Sprites;
    ComponentStorage<VelocityComponent>& m_Velocities;
    ComponentStorage<InputComponent>& m_Inputs;
    ComponentStorage<CharacterStateComponent>& m_CharacterStates;
    ComponentStorage<AnimationControllerComponent>& m_AnimationControllers;
    ComponentStorage<CollisionComponent>& m_Collisions;
    ComponentStorage<ConveyorBeltComponent>& m_ConveyorBelts;
    ComponentStorage<InventoryComponent>& m_Inventories;
    ComponentStorage<MachineFluidComponent>& m_MachineFluids;
    ComponentStorage<MachineFluidPortLinkComponent>& m_MachineFluidPortLinks;
    ComponentStorage<MachineComponent>& m_Machines;
    ComponentStorage<MachineInventoryComponent>& m_MachineInventories;
    ComponentStorage<CraftingMachineComponent>& m_CraftingMachines;
    ComponentStorage<MinerComponent>& m_Miners;
    ComponentStorage<InteractionComponent>& m_Interactions;
    AnimationLibrary& m_AnimationLibrary;
    FluidSystem& m_FluidSystem;
    SpriteAtlas* m_FluidAtlas = nullptr;
    std::unordered_map<long long, Entity> m_FluidPipeEntitiesByTile;
    std::unordered_map<long long, Entity> m_FluidTankEntitiesByTile;
    std::unordered_map<long long, Entity> m_FluidPumpEntitiesByTile;
    const FluidDefinition* m_DefaultFluidDefinition = nullptr;
    FluidDefinition m_DebugWater{"water", "Water"};

    float kTileSize = 32.0f;
};
