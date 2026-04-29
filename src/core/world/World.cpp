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

long long World::makeTileKey(int tileX, int tileY) {
    const unsigned long long x = static_cast<unsigned int>(tileX);
    const unsigned long long y = static_cast<unsigned int>(tileY);
    return static_cast<long long>((x << 32) | y);
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

std::optional<Entity> World::tryInteract(const SDL_Event &event) {
    return m_InteractionSystem.handleInput(event, m_Player, m_Positions, m_Interactions);
}

void World::setRenderer(Renderer *renderer) {
    m_Renderer = renderer;
}

void World::placeConveyorBelt(int tileX, int tileY, Direction direction) {
    removeConveyorBelt(tileX, tileY);

    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_MachineInventories, m_CraftingMachines, m_Interactions, m_AnimationLibrary);

    const float worldX = static_cast<float>(tileX * 32);
    const float worldY = static_cast<float>(tileY * 32);
    Entity belt = factory.createConveyorBelt({ worldX, worldY }, m_ConveyorAtlas, direction);
    m_ConveyorEntitiesByTile[makeTileKey(tileX, tileY)] = belt;
}

void World::removeConveyorBelt(int tileX, int tileY) {
    const long long key = makeTileKey(tileX, tileY);
    auto it = m_ConveyorEntitiesByTile.find(key);
    if (it == m_ConveyorEntitiesByTile.end()) {
        return;
    }

    const Entity belt = it->second;
    m_Positions.remove(belt);
    m_Sprites.remove(belt);
    m_ConveyorBelts.remove(belt);
    m_Collisions.remove(belt);
    m_Interactions.remove(belt);
    m_ConveyorEntitiesByTile.erase(it);
}

void World::clearConveyorBelts() {
    std::vector<std::pair<int, int>> tilesToClear;
    tilesToClear.reserve(m_ConveyorEntitiesByTile.size());

    for (const auto& [tileKey, entity] : m_ConveyorEntitiesByTile) {
        const int tileX = static_cast<int>(tileKey >> 32);
        const int tileY = static_cast<int>(tileKey & 0xffffffff);
        tilesToClear.emplace_back(tileX, tileY);
    }

    for (const auto& [tileX, tileY] : tilesToClear) {
        removeConveyorBelt(tileX, tileY);
    }
}

std::vector<std::tuple<int, int, Direction>> World::getConveyorBeltData() const {
    std::vector<std::tuple<int, int, Direction>> conveyors;
    conveyors.reserve(m_ConveyorEntitiesByTile.size());

    for (const auto& [tileKey, entity] : m_ConveyorEntitiesByTile) {
        const auto* belt = m_ConveyorBelts.get(entity);
        if (!belt) {
            continue;
        }

        const int tileX = static_cast<int>(tileKey >> 32);
        const int tileY = static_cast<int>(tileKey & 0xffffffff);
        conveyors.emplace_back(tileX, tileY, belt->direction);
    }

    return conveyors;
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

    auto* inv = m_Inventories.get(m_Player);
    if (inv) {
        inv->inventory.addItem(m_ItemDatabase.getItem("iron_ingot"), 2);
        inv->inventory.addItem(m_ItemDatabase.getItem("copper_ingot"), 4);
        inv->inventory.addItem(m_ItemDatabase.getItem("iron_ingot"), 2);
    }

    LOG_INFO("Loading conveyor atlas...");
    m_ConveyorAtlas.createAtlas(m_Renderer, 32, 32, "assets/conveyor_sprites.png");

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
