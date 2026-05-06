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

void createMachineFluidPorts(EntityManager& entityManager,
                             ComponentStorage<PositionComponent>& positions,
                             ComponentStorage<FluidPortComponent>& fluidPorts,
                             ComponentStorage<MachineFluidPortLinkComponent>& machineFluidPortLinks,
                             Entity machineEntity,
                             Vec2f machinePosition,
                             const std::vector<MachineFluidPortDefinition>& portDefinitions) {
    for (const MachineFluidPortDefinition& portDefinition : portDefinitions) {
        const Entity portEntity = entityManager.createEntity();
        positions.add(portEntity, {{
            machinePosition.x + static_cast<float>(portDefinition.localTileX * 32),
            machinePosition.y + static_cast<float>(portDefinition.localTileY * 32)
        }});
        fluidPorts.add(portEntity, {portDefinition.type, portDefinition.side, portDefinition.maxTransferPerSecond});
        machineFluidPortLinks.add(portEntity, {machineEntity, portDefinition.slotName, portDefinition.type});
        LOG_INFO("EntityFactory: created machine fluid port entity={} machine={} slot={} type={} side={} localTile=({}, {})",
                 portEntity,
                 machineEntity,
                 portDefinition.slotName,
                 static_cast<int>(portDefinition.type),
                 static_cast<int>(portDefinition.side),
                 portDefinition.localTileX,
                 portDefinition.localTileY);
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

    if (!machineDefinition.fluidPorts.empty()) {
        MachineFluidComponent machineFluid;
        for (const MachineFluidPortDefinition& portDefinition : machineDefinition.fluidPorts) {
            MachineFluidSlot slot;
            slot.name = portDefinition.slotName;
            slot.capacity = portDefinition.capacity;
            if (portDefinition.type == FluidPortType::OUTPUT) {
                machineFluid.outputs.push_back(slot);
            } else {
                machineFluid.inputs.push_back(slot);
            }
        }
        m_MachineFluids.add(machine, machineFluid);
        createMachineFluidPorts(m_EntityManager, m_Positions, m_FluidPorts, m_MachineFluidPortLinks, machine, position, machineDefinition.fluidPorts);
        LOG_INFO("EntityFactory: created crafting machine {} entity={} with {} fluid ports",
                 machineDefinition.uniqueName,
                 machine,
                 machineDefinition.fluidPorts.size());
    }

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

Entity EntityFactory::createFluidTankMachine(Vec2f position, Sprite sprite, const FluidTankMachineDefinition& machineDefinition) const {
    Entity tank = m_EntityManager.createEntity();
    const auto width = static_cast<float>(machineDefinition.widthTiles * 32);
    const auto height = static_cast<float>(machineDefinition.heightTiles * 32);

    m_Positions.add(tank, { position });
    m_Sprites.add(tank, { sprite, 0, width, height });
    m_Collisions.add(tank, { SDL_FRect(0.0f, 0.0f, width, height), true, false });
    m_Machines.add(tank, { machineDefinition.uniqueName });
    m_FluidTanks.add(tank, { {}, machineDefinition.capacity });

    InteractionComponent interaction;
    interaction.interactionName = machineDefinition.displayName.empty() ? "Fluid Tank" : machineDefinition.displayName;
    interaction.interactionBounds = {-8.0f, -8.0f, width + 16.0f, height + 16.0f};
    m_Interactions.add(tank, interaction);

    return tank;
}

Entity EntityFactory::createFluidPumpMachine(Vec2f position,
                                             Sprite sprite,
                                             const FluidPumpMachineDefinition& machineDefinition,
                                             Direction outputDirection,
                                             const FluidDefinition* outputFluid) const {
    Entity pump = m_EntityManager.createEntity();
    const auto width = static_cast<float>(machineDefinition.widthTiles * 32);
    const auto height = static_cast<float>(machineDefinition.heightTiles * 32);

    m_Positions.add(pump, { position });
    m_Sprites.add(pump, { sprite, 0, width, height });
    m_Collisions.add(pump, { SDL_FRect(0.0f, 0.0f, width, height), true, false });
    m_Machines.add(pump, { machineDefinition.uniqueName });
    m_FluidPumps.add(pump, { outputFluid, machineDefinition.outputPerSecond });
    m_FluidPorts.add(pump, { FluidPortType::OUTPUT, outputDirection, machineDefinition.outputPerSecond });

    InteractionComponent interaction;
    interaction.interactionName = machineDefinition.displayName.empty() ? "Fluid Pump" : machineDefinition.displayName;
    interaction.interactionBounds = {-8.0f, -8.0f, width + 16.0f, height + 16.0f};
    m_Interactions.add(pump, interaction);

    return pump;
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

Entity EntityFactory::createFluidPipe(Vec2f position, Sprite sprite, Direction direction) const {
    Entity pipe = m_EntityManager.createEntity();
    m_Positions.add(pipe, { position });
    m_Sprites.add(pipe, { sprite, 0, 32.0f, 32.0f, false, true });

    FluidPipeComponent pipeComponent;
    pipeComponent.direction = direction;
    const bool isVertical = direction == Direction::UP || direction == Direction::DOWN;
    pipeComponent.connectUp = isVertical;
    pipeComponent.connectDown = isVertical;
    pipeComponent.connectLeft = !isVertical;
    pipeComponent.connectRight = !isVertical;
    m_FluidPipes.add(pipe, pipeComponent);
    return pipe;
}

Entity EntityFactory::createFluidTank(Vec2f position, Sprite sprite) const {
    Entity tank = m_EntityManager.createEntity();
    m_Positions.add(tank, { position });
    m_Sprites.add(tank, { sprite, 0, 32.0f, 32.0f, false, true });
    m_FluidTanks.add(tank, {});
    return tank;
}

Entity EntityFactory::createFluidPump(Vec2f position, Sprite sprite, Direction outputDirection, const FluidDefinition* outputFluid, float outputPerSecond) const {
    Entity pump = m_EntityManager.createEntity();
    m_Positions.add(pump, { position });
    m_Sprites.add(pump, { sprite, 0, 32.0f, 32.0f, false, true });
    m_FluidPumps.add(pump, { outputFluid, outputPerSecond });
    m_FluidPorts.add(pump, { FluidPortType::OUTPUT, outputDirection, outputPerSecond });
    return pump;
}
