#pragma once
#include "AnimationLibrary.hpp"
#include "../machine/MachineDefinition.hpp"
#include "../entities/ComponentStorage.hpp"
#include "../entities/EntityManager.hpp"
#include "../entities/component/AnimationControllerComponent.hpp"
#include "../entities/component/CharacterStateComponent.hpp"
#include "../entities/component/CollisionComponent.hpp"
#include "../entities/component/ConveyorBeltComponent.hpp"
#include "../entities/component/InputComponent.hpp"
#include "../entities/component/InteractionComponent.hpp"
#include "../entities/component/InventoryComponent.hpp"
#include "../entities/component/FluidPipeComponent.hpp"
#include "../entities/component/FluidPortComponent.hpp"
#include "../entities/component/FluidPumpComponent.hpp"
#include "../entities/component/FluidTankComponent.hpp"
#include "../entities/component/MachineFluidComponent.hpp"
#include "../entities/component/MachineFluidPortLinkComponent.hpp"
#include "../entities/component/MachineComponent.hpp"
#include "../entities/component/MinerComponent.hpp"
#include "../entities/component/PositionComponent.hpp"
#include "../entities/component/SpriteComponent.hpp"
#include "../entities/component/VelocityComponent.hpp"

class SpriteAtlas;
struct CraftingMachineComponent;
struct MachineInventoryComponent;

class EntityFactory {
public:
    EntityFactory(EntityManager &entityManager, ComponentStorage<PositionComponent> &positions,
                  ComponentStorage<VelocityComponent> &velocities,
                  ComponentStorage<InputComponent> &inputs,
                  ComponentStorage<CharacterStateComponent> &characterStates,
                  ComponentStorage<AnimationControllerComponent> &animationControllers,
                  ComponentStorage<SpriteComponent> &sprites,
                  ComponentStorage<CollisionComponent> &collisions,
                  ComponentStorage<ConveyorBeltComponent> &conveyorBelts,
                  ComponentStorage<InventoryComponent> &inventories,
                  ComponentStorage<FluidPipeComponent> &fluidPipes,
                  ComponentStorage<FluidTankComponent> &fluidTanks,
                  ComponentStorage<FluidPumpComponent> &fluidPumps,
                  ComponentStorage<FluidPortComponent> &fluidPorts,
                  ComponentStorage<MachineFluidComponent>& machineFluids,
                  ComponentStorage<MachineFluidPortLinkComponent>& machineFluidPortLinks,
                  ComponentStorage<MachineComponent> &machines,
                  ComponentStorage<MachineInventoryComponent> &machineInventories,
                  ComponentStorage<CraftingMachineComponent> &craftingMachines,
                  ComponentStorage<MinerComponent> &miners,
                  ComponentStorage<InteractionComponent> &interactions,
                  AnimationLibrary &animationLibrary) : m_EntityManager(entityManager),
                                                        m_Positions(positions),
                                                        m_Velocities(velocities),
                                                        m_Inputs(inputs),
                                                        m_CharacterStates(characterStates),
                                                        m_AnimationControllers(animationControllers),
                                                        m_Sprites(sprites),
                                                        m_Collisions(collisions),
                                                        m_ConveyorBelts(conveyorBelts),
                                                        m_Inventories(inventories),
                                                        m_FluidPipes(fluidPipes),
                                                        m_FluidTanks(fluidTanks),
                                                        m_FluidPumps(fluidPumps),
                                                        m_FluidPorts(fluidPorts),
                                                        m_MachineFluids(machineFluids),
                                                        m_MachineFluidPortLinks(machineFluidPortLinks),
                                                        m_Machines(machines),
                                                        m_MachineInventories(machineInventories),
                                                        m_CraftingMachines(craftingMachines),
                                                        m_Miners(miners),
                                                        m_Interactions(interactions),
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

private:
    EntityManager &m_EntityManager;

    ComponentStorage<PositionComponent> &m_Positions;
    ComponentStorage<VelocityComponent> &m_Velocities;
    ComponentStorage<InputComponent> &m_Inputs;
    ComponentStorage<CharacterStateComponent> &m_CharacterStates;
    ComponentStorage<AnimationControllerComponent> &m_AnimationControllers;
    ComponentStorage<SpriteComponent> &m_Sprites;
    ComponentStorage<CollisionComponent> &m_Collisions;
    ComponentStorage<ConveyorBeltComponent> &m_ConveyorBelts;
    ComponentStorage<InventoryComponent> &m_Inventories;
    ComponentStorage<FluidPipeComponent> &m_FluidPipes;
    ComponentStorage<FluidTankComponent> &m_FluidTanks;
    ComponentStorage<FluidPumpComponent> &m_FluidPumps;
    ComponentStorage<FluidPortComponent> &m_FluidPorts;
    ComponentStorage<MachineFluidComponent>& m_MachineFluids;
    ComponentStorage<MachineFluidPortLinkComponent>& m_MachineFluidPortLinks;
    ComponentStorage<MachineComponent> &m_Machines;
    ComponentStorage<MachineInventoryComponent> &m_MachineInventories;
    ComponentStorage<CraftingMachineComponent> &m_CraftingMachines;
    ComponentStorage<MinerComponent> &m_Miners;
    ComponentStorage<InteractionComponent> &m_Interactions;
    AnimationLibrary &m_AnimationLibrary;
};
