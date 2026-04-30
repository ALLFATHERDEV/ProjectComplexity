#pragma once

#include "AnimationLibrary.hpp"
#include "../entities/ComponentStorage.hpp"
#include "../entities/EntityManager.hpp"
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
#include "../graphics/SpriteAtlas.hpp"

#include <tuple>
#include <unordered_map>
#include <vector>

struct AnimationControllerComponent;

class ConveyorManager {
public:
    ConveyorManager(EntityManager& entityManager,
                    ComponentStorage<PositionComponent>& positions,
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
                    AnimationLibrary& animationLibrary);

    void setAtlas(SpriteAtlas* atlas);
    void placeConveyorBelt(int tileX, int tileY, Direction direction);
    void removeConveyorBelt(int tileX, int tileY);
    void clearConveyorBelts();
    std::vector<std::tuple<int, int, Direction>> getConveyorBeltData() const;

private:
    static long long makeTileKey(int tileX, int tileY);
    Entity getConveyorBeltAt(int tileX, int tileY) const;
    void refreshConveyorSpriteAt(int tileX, int tileY);
    void refreshConveyorSpritesAround(int tileX, int tileY);

    EntityManager& m_EntityManager;
    ComponentStorage<PositionComponent>& m_Positions;
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
    SpriteAtlas* m_ConveyorAtlas = nullptr;
    std::unordered_map<long long, Entity> m_ConveyorEntitiesByTile;
};
