#include "EntityFactory.hpp"

#include "../Logger.hpp"
#include "../entities/component/CraftingMachineComponent.hpp"
#include "../entities/component/ConveyorBeltComponent.hpp"
#include "../entities/component/InteractionComponent.hpp"
#include "../entities/component/InventoryComponent.hpp"
#include "../entities/component/MachineComponent.hpp"
#include "../entities/component/MachineInventoryComponent.hpp"
#include "../entities/component/MinerComponent.hpp"
#include "../graphics/SpriteAtlas.hpp"

Sprite getConveyorSpriteForDirection(const SpriteAtlas& atlas, Direction direction) {
    switch (direction) {
        case Direction::DOWN:
            return atlas.getSprite(0, 6);
        case Direction::UP:
            return atlas.getSprite(2, 6);
        case Direction::LEFT:
            return atlas.getSprite(7, 5);
        case Direction::RIGHT:
            return atlas.getSprite(0, 5);
        default:
            return atlas.getSprite(0, 0);
    }
}

Entity EntityFactory::createPlayer(Vec2f position) const {
    Entity player = m_EntityManager.createEntity();
    m_Positions.add(player, { position });
    m_Velocities.add(player, { 0.0f, 0.0f});
    m_Inputs.add(player, { 250 });
    m_CharacterStates.add(player, {CharacterState::IDLE, Direction::DOWN} );
    m_Collisions.add(player, { SDL_FRect(4.0f, 8.0f, 24.0f, 24.0f), true, false });

    AnimationControllerComponent controller;
    controller.animations[{ "idle", true, Direction::DOWN }] = *m_AnimationLibrary.get("player_idle_down");
    controller.animations[{ "idle", true, Direction::UP }] = *m_AnimationLibrary.get("player_idle_up");
    controller.animations[{ "idle", true, Direction::RIGHT }] = *m_AnimationLibrary.get("player_idle_right");
    controller.animations[{ "idle", true, Direction::LEFT }] = *m_AnimationLibrary.get("player_idle_left");
    controller.animations[{ "walk", true, Direction::LEFT }] = *m_AnimationLibrary.get("player_walk_left");
    controller.animations[{ "walk", true, Direction::RIGHT }] = *m_AnimationLibrary.get("player_walk_right");
    controller.animations[{ "walk", true, Direction::UP }] = *m_AnimationLibrary.get("player_walk_up");
    controller.animations[{ "walk", true, Direction::DOWN }] = *m_AnimationLibrary.get("player_walk_down");

    controller.stateName = "idle";
    controller.useDirection = true;
    controller.direction = Direction::DOWN;
    controller.sortOrder = 10;
    auto idleDownIt = controller.animations.find({"idle", true, Direction::DOWN});
    if (idleDownIt != controller.animations.end()) {
        controller.currentAnimation = &idleDownIt->second;
        controller.currentAnimation->play();
    }
    m_AnimationControllers.add(player, controller);

    InventoryComponent inventory;
    inventory.inventory.create(10, 4);
    m_Inventories.add(player, inventory);

    return player;
}

Entity EntityFactory::createCraftingMachine(Vec2f position, Sprite sprite, const MachineDefinition& machineDefinition) const {
    Entity machine = m_EntityManager.createEntity();
    const auto width = static_cast<float>(machineDefinition.widthTiles * 32);
    const auto height = static_cast<float>(machineDefinition.heightTiles * 32);

    m_Positions.add(machine, { position });
    m_Sprites.add(machine, { sprite, 0, width, height });
    m_Collisions.add(machine, { SDL_FRect(0.0f, 0.0f, width, height), true, false });
    m_Machines.add(machine, { machineDefinition.uniqueName });

    MachineInventoryComponent inventory;
    inventory.fuelInventory.create(machineDefinition.fuelWidth, machineDefinition.fuelHeight);
    inventory.inputInventory.create(1, 1);
    inventory.outputInventory.create(1, 1);
    m_MachineInventories.add(machine, inventory);

    CraftingMachineComponent crafting;
    crafting.machineUniqueName = machineDefinition.uniqueName;
    crafting.availableRecipes = machineDefinition.availableRecipes;
    crafting.requiresFuel = machineDefinition.requiresFuel;
    if (!crafting.availableRecipes.empty()) {
        crafting.currentRecipeName = crafting.availableRecipes.front();
    }
    m_CraftingMachines.add(machine, crafting);

    InteractionComponent interaction;
    interaction.interactionName = machineDefinition.displayName.empty() ? "Crafting Machine" : machineDefinition.displayName;
    interaction.interactionBounds = {-8.0f, -8.0f, width + 16.0f, height + 16.0f};
    m_Interactions.add(machine, interaction);

    return machine;
}

Entity EntityFactory::createMiner(Vec2f position, Sprite sprite, const MinerMachineDefinition& machineDefinition) const {
    Entity miner = m_EntityManager.createEntity();
    const auto width = static_cast<float>(machineDefinition.widthTiles * 32);
    const auto height = static_cast<float>(machineDefinition.heightTiles * 32);

    m_Positions.add(miner, { position });
    m_Sprites.add(miner, { sprite, 0, width, height });
    m_Collisions.add(miner, { SDL_FRect(0.0f, 0.0f, width, height), true, false });
    m_Machines.add(miner, { machineDefinition.uniqueName });

    MachineInventoryComponent inventory;
    inventory.fuelInventory.create(machineDefinition.fuelWidth, machineDefinition.fuelHeight);
    inventory.outputInventory.create(1, 1);
    m_MachineInventories.add(miner, inventory);

    MinerComponent minerComponent;
    minerComponent.machineUniqueName = machineDefinition.uniqueName;
    minerComponent.requiresFuel = machineDefinition.requiresFuel;
    minerComponent.miningSpeed = machineDefinition.miningSpeed;
    minerComponent.widthTiles = machineDefinition.widthTiles;
    minerComponent.heightTiles = machineDefinition.heightTiles;
    m_Miners.add(miner, minerComponent);

    InteractionComponent interaction;
    interaction.interactionName = machineDefinition.displayName.empty() ? "Miner" : machineDefinition.displayName;
    interaction.interactionBounds = {-8.0f, -8.0f, width + 16.0f, height + 16.0f};
    m_Interactions.add(miner, interaction);

    return miner;
}

Entity EntityFactory::createConveyorBelt(Vec2f position, const SpriteAtlas& atlas, Direction direction) const {
    Entity belt = m_EntityManager.createEntity();

    Sprite sprite = getConveyorSpriteForDirection(atlas, direction);

    m_Positions.add(belt, { position });
    m_Sprites.add(belt, { sprite, 0, 0.0f, 0.0f, false, false });
    m_ConveyorBelts.add(belt, { direction });

    AnimationControllerComponent controller;
    if (const auto* animation = m_AnimationLibrary.get("right")) controller.animations[{"default", true, Direction::RIGHT}] = *animation;
    if (const auto* animation = m_AnimationLibrary.get("down")) controller.animations[{"default", true, Direction::DOWN}] = *animation;
    if (const auto* animation = m_AnimationLibrary.get("left")) controller.animations[{"default", true, Direction::LEFT}] = *animation;
    if (const auto* animation = m_AnimationLibrary.get("up")) controller.animations[{"default", true, Direction::UP}] = *animation;
    controller.stateName = "default";
    controller.useDirection = true;
    controller.direction = direction;
    controller.sortOrder = 0;
    auto directionIt = controller.animations.find({"default", true, direction});
    if (directionIt != controller.animations.end()) {
        controller.currentAnimation = &directionIt->second;
        controller.currentAnimation->play();
    }
    m_AnimationControllers.add(belt, controller);

    return belt;
}

Entity EntityFactory::createStorageContainer(Vec2f position, Sprite sprite, Vec2i containerInventorySize, Vec2i sizeTiles, bool isBlocking) const {
    Entity container = m_EntityManager.createEntity();
    const float width = static_cast<float>(sizeTiles.x * 32);
    const float height = static_cast<float>(sizeTiles.y * 32);
    m_Positions.add(container, { position });
    m_Sprites.add(container, { sprite, 0, width, height });
    m_Collisions.add(container, { SDL_FRect(0.0f, 0.0f, width, height), isBlocking, false });

    InventoryComponent inventory;
    inventory.inventory.create(containerInventorySize.x, containerInventorySize.y);
    m_Inventories.add(container, inventory);

    InteractionComponent interaction;
    interaction.interactionName = "Storage Container";
    interaction.interactionBounds = {-8.0f, -8.0f, width + 16.0f, height + 16.0f};
    m_Interactions.add(container, interaction);
    return container;
}
