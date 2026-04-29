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
#include "../entities/component/PositionComponent.hpp"
#include "../entities/component/SpriteComponent.hpp"
#include "../entities/component/VelocityComponent.hpp"
#include <vector>

class SpriteAtlas;
struct CraftingMachineComponent;
struct MachineInventoryComponent;

class EntityFactory {
public:
    EntityFactory(EntityManager& entityManager, ComponentStorage<PositionComponent>& positions,
                    ComponentStorage<VelocityComponent>& velocities,
                    ComponentStorage<InputComponent>& inputs,
                    ComponentStorage<CharacterStateComponent>& characterStates,
                    ComponentStorage<AnimationControllerComponent>& animationControllers,
                    ComponentStorage<SpriteComponent>& sprites,
                    ComponentStorage<CollisionComponent>& collisions,
                    ComponentStorage<ConveyorBeltComponent>& conveyorBelts,
                    ComponentStorage<InventoryComponent>& inventories,
                    ComponentStorage<MachineInventoryComponent>& machineInventories,
                    ComponentStorage<CraftingMachineComponent>& craftingMachines,
                    ComponentStorage<InteractionComponent>& interactions,
                    AnimationLibrary& animationLibrary) :
    m_EntityManager(entityManager),
    m_Positions(positions),
    m_Velocities(velocities),
    m_Inputs(inputs),
    m_CharacterStates(characterStates),
    m_AnimationControllers(animationControllers),
    m_Sprites(sprites),
    m_Collisions(collisions),
    m_ConveyorBelts(conveyorBelts),
    m_Inventories(inventories),
    m_MachineInventories(machineInventories),
    m_CraftingMachines(craftingMachines),
    m_Interactions(interactions),
    m_AnimationLibrary(animationLibrary) {}

    Entity createPlayer(Vec2f position);
    Entity createCraftingMachine(Vec2f position, Sprite sprite, const MachineDefinition& machineDefinition);
    Entity createConveyorBelt(Vec2f position, const SpriteAtlas& atlas, Direction direction);

private:
    EntityManager& m_EntityManager;

    ComponentStorage<PositionComponent>& m_Positions;
    ComponentStorage<VelocityComponent>& m_Velocities;
    ComponentStorage<InputComponent>& m_Inputs;
    ComponentStorage<CharacterStateComponent>& m_CharacterStates;
    ComponentStorage<AnimationControllerComponent>& m_AnimationControllers;
    ComponentStorage<SpriteComponent>& m_Sprites;
    ComponentStorage<CollisionComponent>& m_Collisions;
    ComponentStorage<ConveyorBeltComponent>& m_ConveyorBelts;
    ComponentStorage<InventoryComponent>& m_Inventories;
    ComponentStorage<MachineInventoryComponent>& m_MachineInventories;
    ComponentStorage<CraftingMachineComponent>& m_CraftingMachines;
    ComponentStorage<InteractionComponent>& m_Interactions;
    AnimationLibrary& m_AnimationLibrary;
};
