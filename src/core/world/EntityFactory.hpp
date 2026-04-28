#pragma once
#include "AnimationLibrary.hpp"
#include "../entities/ComponentStorage.hpp"
#include "../entities/EntityManager.hpp"
#include "../entities/component/AnimationControllerComponent.hpp"
#include "../entities/component/CharacterStateComponent.hpp"
#include "../entities/component/CollisionComponent.hpp"
#include "../entities/component/InputComponent.hpp"
#include "../entities/component/InventoryComponent.hpp"
#include "../entities/component/PositionComponent.hpp"
#include "../entities/component/SpriteComponent.hpp"
#include "../entities/component/VelocityComponent.hpp"

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
                    ComponentStorage<InventoryComponent>& inventories,
                    ComponentStorage<MachineInventoryComponent>& machineInventories,
                    ComponentStorage<CraftingMachineComponent>& craftingMachines,
                    AnimationLibrary& animationLibrary) :
    m_EntityManager(entityManager),
    m_Positions(positions),
    m_Velocities(velocities),
    m_Inputs(inputs),
    m_CharacterStates(characterStates),
    m_AnimationControllers(animationControllers),
    m_Sprites(sprites),
    m_Collisions(collisions),
    m_Inventories(inventories),
    m_MachineInventories(machineInventories),
    m_CraftingMachines(craftingMachines),
    m_AnimationLibrary(animationLibrary) {}

    Entity createPlayer(Vec2f position);
    Entity createCraftingMachine(Vec2f position, Sprite sprite);

private:
    EntityManager& m_EntityManager;

    ComponentStorage<PositionComponent>& m_Positions;
    ComponentStorage<VelocityComponent>& m_Velocities;
    ComponentStorage<InputComponent>& m_Inputs;
    ComponentStorage<CharacterStateComponent>& m_CharacterStates;
    ComponentStorage<AnimationControllerComponent>& m_AnimationControllers;
    ComponentStorage<SpriteComponent>& m_Sprites;
    ComponentStorage<CollisionComponent>& m_Collisions;
    ComponentStorage<InventoryComponent>& m_Inventories;
    ComponentStorage<MachineInventoryComponent>& m_MachineInventories;
    ComponentStorage<CraftingMachineComponent>& m_CraftingMachines;
    AnimationLibrary& m_AnimationLibrary;
};
