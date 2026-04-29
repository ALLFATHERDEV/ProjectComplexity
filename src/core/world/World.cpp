#include "World.hpp"

#include "EntityFactory.hpp"
#include "../Game.hpp"
#include "../Logger.hpp"
#include "../editor/TileMapSerializer.hpp"
#include "../graphics/AnimationLoader.hpp"
#include "../graphics/SpriteAtlas.hpp"

World::World()
    : m_ConveyorManager(m_EntityManager,
                        m_Positions,
                        m_Sprites,
                        m_Velocities,
                        m_Inputs,
                        m_CharacterStates,
                        m_AnimationControllers,
                        m_Collisions,
                        m_ConveyorBelts,
                        m_Inventories,
                        m_MachineInventories,
                        m_CraftingMachines,
                        m_Interactions,
                        m_AnimationLibrary) {

}

World::~World() {
}

void World::update(float deltaTime) {
    m_MovementInputSystem.update(m_Inputs, m_Velocities);
    m_CollisionSystem.update(deltaTime, m_Positions, m_Velocities, m_Collisions, m_TileMap);

    m_CharacterStateSystem.update(m_CharacterStates, m_Velocities);
    m_AnimationStateSystem.update(m_CharacterStates, m_AnimationControllers);
    m_AnimationSystem.update(deltaTime, m_AnimationControllers);
    // m_MovementSystem.update(deltaTime, m_Positions, m_Velocities);
    m_CraftingSystem.update(deltaTime, m_CraftingMachines, m_MachineInventories, m_RecipeDatabase, m_ItemDatabase);
    m_ConveyorSystem.update(deltaTime, m_EntityManager, m_Positions, m_Sprites, m_ConveyorBelts, m_ConveyorItems, m_MachineInventories);

    auto* playerPos = m_Positions.get(m_Player);
    if (playerPos) {
        m_ChunkManager.update(playerPos->position, 32);
        const float halfWidth = static_cast<float>(Game::WINDOW_WIDTH) / (2.0f * m_Camera.getZoom());
        const float halfHeight = static_cast<float>(Game::WINDOW_HEIGHT) / (2.0f * m_Camera.getZoom());
        m_Camera.setPosition({playerPos->position.x - halfWidth, playerPos->position.y - halfHeight});
    }

}

void World::render() {
    m_TileMap.render(m_Renderer, m_Camera, m_ChunkManager);

    m_RenderSystem.render(m_Renderer, m_Camera, m_ChunkManager, m_Positions, m_Sprites);
    m_AnimatedRenderSystem.render(m_Renderer, m_Camera, m_ChunkManager, m_Positions, m_AnimationControllers);
}

void World::handleInput(SDL_Event &event) {
    m_InputSystem.handleInput(event, m_Inputs, m_Velocities);

    m_InteractionSystem.handleInput(event, m_Player, m_Positions, m_Interactions, m_ChunkManager);
}

std::optional<Entity> World::tryInteract(const SDL_Event &event) {
    return m_InteractionSystem.handleInput(event, m_Player, m_Positions, m_Interactions, m_ChunkManager);
}

void World::setRenderer(Renderer *renderer) {
    m_Renderer = renderer;
}

void World::placeConveyorBelt(int tileX, int tileY, Direction direction) {
    m_ConveyorManager.placeConveyorBelt(tileX, tileY, direction);
}

void World::removeConveyorBelt(int tileX, int tileY) {
    m_ConveyorManager.removeConveyorBelt(tileX, tileY);
}

void World::clearConveyorBelts() {
    m_ConveyorManager.clearConveyorBelts();
}

std::vector<std::tuple<int, int, Direction>> World::getConveyorBeltData() const {
    return m_ConveyorManager.getConveyorBeltData();
}

void World::initializeWorld() {

    LOG_INFO("Loading animations...");
    AnimationLoader::createSpriteAtlas(m_Renderer);
    AnimationLoader::loadPlayerAnimations(m_AnimationLibrary);

    LOG_INFO("Loading items...");
    m_ItemAtlas.createAtlas(m_Renderer, 32, 32, "assets/spritesheets/items_sheet.png");
    m_ItemDatabase.loadItemsFromFolder("assets/items", m_ItemAtlas);

    LOG_INFO("Loading recipes...");
    m_RecipeDatabase.loadRecipesFromFolder("assets/recipes");

    LOG_INFO("Loading machines...");
    m_MachineDatabase.loadMachinesFromFolder("assets/machines");

    LOG_INFO("Creating entities...");
    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_MachineInventories, m_CraftingMachines, m_Interactions, m_AnimationLibrary);

    m_Player = factory.createPlayer({ 100.0f, 100.0f });
    m_ChunkManager.update({100.0f, 100.0f}, 32);

    auto* inv = m_Inventories.get(m_Player);
    if (inv) {
        inv->inventory.addItem(m_ItemDatabase.getItem("iron_ingot"), 2);
        inv->inventory.addItem(m_ItemDatabase.getItem("copper_ingot"), 4);
        inv->inventory.addItem(m_ItemDatabase.getItem("iron_ingot"), 2);
    }

    LOG_INFO("Loading conveyor atlas...");
    m_ConveyorAtlas.createAtlas(m_Renderer, 32, 32, "assets/conveyor_sprites.png");
    m_ConveyorManager.setAtlas(&m_ConveyorAtlas);

    LOG_INFO("Creating map...");
    m_TileMapAtlas.createAtlas(m_Renderer, 16, 16, "assets/Overworld_Tileset.png");
    TileMapSerializer::load(*this, "maps/test.map");


    const MachineDefinition* basicCrafter = m_MachineDatabase.getMachine("basic_crafter");
    if (!basicCrafter) {
        LOG_ERROR("Missing machine definition: basic_crafter");
        return;
    }

    m_Machine = factory.createCraftingMachine({ 100.0f, 100.0f }, m_TileMapAtlas.getSprite(0, 3), *basicCrafter);

    const ItemDefinition* iron = m_ItemDatabase.getItem("iron_ingot");
    auto* machineInventory = m_MachineInventories.get(m_Machine);
    if (iron && machineInventory) {
        machineInventory->inputInventory.addItem(iron, 4);
    }

    LOG_INFO("World initialized");
}
