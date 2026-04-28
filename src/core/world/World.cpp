#include "World.hpp"

#include "EntityFactory.hpp"
#include "../Game.hpp"
#include "../Logger.hpp"
#include "../editor/TileMapSerializer.hpp"
#include "../graphics/AnimationLoader.hpp"
#include "../graphics/SpriteAtlas.hpp"

World::World() {

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

    auto* playerPos = m_Positions.get(m_Player);
    if (playerPos) {
        m_Camera.setPosition({playerPos->position.x - (Game::WINDOW_WIDTH / 2), playerPos->position.y - (Game::WINDOW_HEIGHT / 2)});
    }

}

void World::render() {
    m_TileMap.render(m_Renderer, m_Camera);

    m_RenderSystem.render(m_Renderer, m_Camera, m_Positions, m_Sprites);
    m_AnimatedRenderSystem.render(m_Renderer, m_Camera, m_Positions, m_AnimationControllers);
}

void World::handleInput(SDL_Event &event) {
    m_InputSystem.handleInput(event, m_Inputs, m_Velocities);

    m_InteractionSystem.handleInput(event, m_Player, m_Positions, m_Interactions);
}

void World::setRenderer(Renderer *renderer) {
    m_Renderer = renderer;
}

void World::initializeWorld() {

    LOG_INFO("Loading animations...");
    AnimationLoader::createSpriteAtlas(m_Renderer);
    AnimationLoader::loadPlayerAnimations(m_AnimationLibrary);

    LOG_INFO("Loading items...");
    m_ItemAtlas.createAtlas(m_Renderer, 32, 32, "assets/spritesheets/items_sheet.png");
    m_ItemDatabase.loadItemsFromFolder("assets/items", m_ItemAtlas);

    LOG_INFO("Creating entities...");
    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_Inventories, m_MachineInventories, m_CraftingMachines, m_Interactions, m_AnimationLibrary);

    m_Player = factory.createPlayer({ 100.0f, 100.0f });

    auto* inv = m_Inventories.get(m_Player);
    if (inv) {
        inv->inventory.addItem(m_ItemDatabase.getItem("iron_ingot"), 2);
        inv->inventory.addItem(m_ItemDatabase.getItem("copper_ingot"), 4);
        inv->inventory.addItem(m_ItemDatabase.getItem("iron_ingot"), 2);
    }

    LOG_INFO("Creating map...");
    m_TileMapAtlas.createAtlas(m_Renderer, 16, 16, "assets/Overworld_Tileset.png");
    TileMapSerializer::load(m_TileMap, m_TileMapAtlas, "maps/test.map");

    RecipeDefinition testRecipe;
    testRecipe.uniqueName = "test_recipe";
    testRecipe.inputs.push_back({ "iron_ingot", 2 });
    testRecipe.outputItemName = "copper_ingot";
    testRecipe.outputAmount = 4;
    testRecipe.craftTime = 3.0f;
    m_RecipeDatabase.addRecipe(testRecipe);

    m_Machine = factory.createCraftingMachine({ 100.0f, 100.0f }, m_TileMapAtlas.getSprite(0, 3));

    const ItemDefinition* iron = m_ItemDatabase.getItem("iron_ingot");
    auto* machineInventory = m_MachineInventories.get(m_Machine);
    if (iron && machineInventory) {
        machineInventory->inputInventory.addItem(iron, 3);
    }

    LOG_INFO("World initialized");
}

