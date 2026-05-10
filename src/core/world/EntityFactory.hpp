#pragma once
#include "AnimationLibrary.hpp"
#include "../entities/ComponentRegistry.hpp"
#include "../machine/MachineDefinition.hpp"
#include "../entities/EntityManager.hpp"
#include "../entities/component/CharacterStateComponent.hpp"
#include "../entities/component/FluidTankComponent.hpp"
#include "../entities/component/VelocityComponent.hpp"

class SpriteAtlas;
struct CraftingMachineComponent;
struct MachineInventoryComponent;

class EntityFactory {
public:
    EntityFactory(EntityManager &entityManager, ComponentRegistry& componentRegistry,
                  AnimationLibrary &animationLibrary)
        : m_EntityManager(entityManager),
          m_Components(componentRegistry),
          m_AnimationLibrary(animationLibrary) {
    }

    Entity createPlayer(Vec2f position) const;
    Entity createCraftingMachine(Vec2f position, Sprite sprite, const MachineDefinition &machineDefinition) const;
    Entity createMiner(Vec2f position, Sprite sprite, const MinerMachineDefinition &machineDefinition) const;
    Entity createFluidTankMachine(Vec2f position, Sprite sprite, const FluidTankMachineDefinition& machineDefinition) const;
    Entity createFluidPumpMachine(Vec2f position, Sprite sprite, const FluidPumpMachineDefinition& machineDefinition, Direction outputDirection, const FluidDefinition* outputFluid) const;
    Entity createConveyorBelt(Vec2f position, const SpriteAtlas &atlas, Direction direction) const;
    Entity createStorageContainer(Vec2f position, Sprite sprite, Vec2i containerInventorySize, Vec2i sizeTiles, bool isBlocking) const;
    Entity createFluidPipe(Vec2f position, Sprite sprite, Direction direction) const;
    Entity createFluidTank(Vec2f position, Sprite sprite) const;
    Entity createFluidPump(Vec2f position, Sprite sprite, Direction outputDirection, const FluidDefinition *outputFluid, float outputPerSecond) const;
    Entity createHaulerBot(Vec2f position, Sprite sprite) const;

private:
    EntityManager &m_EntityManager;
    ComponentRegistry& m_Components;
    AnimationLibrary &m_AnimationLibrary;
};
