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

private:
    static long long makeTileKey(int tileX, int tileY);
    Entity getFluidPipeAtTile(int tileX, int tileY) const;
    Entity getFluidTankAtTile(int tileX, int tileY) const;
    Entity getFluidPumpAtTile(int tileX, int tileY) const;
    void refreshFluidSpriteAt(int tileX, int tileY);
    void refreshFluidSpritesAround(int tileX, int tileY);
    Sprite getPipeSprite(Direction direction) const;
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
    FluidDefinition m_DebugWater{"water", "Water"};

    float kTileSize = 32.0f;
};
