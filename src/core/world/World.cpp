#include "World.hpp"

#include <algorithm>

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
                        m_MachineEntities,
                        m_MachineInventories,
                        m_CraftingMachines,
                        m_Miners,
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
    m_MiningSystem.update(deltaTime, m_Miners, m_Positions, m_MachineInventories, m_TileMap, m_TileMetadataDatabase, m_ItemDatabase);
    m_ConveyorSystem.update(deltaTime, m_EntityManager, m_Positions, m_Sprites, m_ConveyorBelts, m_ConveyorItems, m_MachineInventories);

    auto* playerPos = m_Positions.get(m_Player);
    if (playerPos) {
        m_ChunkManager.update(playerPos->position, 32);
        const float halfWidth = static_cast<float>(Game::WINDOW_WIDTH) / (2.0f * m_Camera.getZoom());
        const float halfHeight = static_cast<float>(Game::WINDOW_HEIGHT) / (2.0f * m_Camera.getZoom());

        float cameraX = playerPos->position.x - halfWidth;
        float cameraY = playerPos->position.y - halfHeight;

        int mapPixelWidth = 0;
        int mapPixelHeight = 0;
        for (const TileMapLayer& layer : m_TileMap.getLayers()) {
            mapPixelWidth = std::max(mapPixelWidth, layer.getWidth() * layer.getCellWidth());
            mapPixelHeight = std::max(mapPixelHeight, layer.getHeight() * layer.getCellHeight());
        }

        const float viewportWidth = halfWidth * 2.0f;
        const float viewportHeight = halfHeight * 2.0f;

        const float maxCameraX = std::max(0.0f, static_cast<float>(mapPixelWidth) - viewportWidth);
        const float maxCameraY = std::max(0.0f, static_cast<float>(mapPixelHeight) - viewportHeight);

        cameraX = std::clamp(cameraX, 0.0f, maxCameraX);
        cameraY = std::clamp(cameraY, 0.0f, maxCameraY);

        m_Camera.setPosition({cameraX, cameraY});
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

void World::registerTilePalette(const std::string& name, SpriteAtlas* atlas) {
    if (!atlas) {
        return;
    }

    m_TilePalettes.push_back({name, atlas});
}

SpriteAtlas* World::getTilePalette(const std::string& name) const {
    for (const auto& palette : m_TilePalettes) {
        if (palette.name == name) {
            return palette.atlas;
        }
    }

    return nullptr;
}

Vec2f World::getPlayerPosition() const {
    const PositionComponent* playerPosition = m_Positions.get(m_Player);
    if (!playerPosition) {
        return {0.0f, 0.0f};
    }

    return playerPosition->position;
}

Sprite World::getMachineSprite(const MachineDefinition& machineDefinition) const {
    return m_TileMapAtlas.getSprite(machineDefinition.spriteAtlasX, machineDefinition.spriteAtlasY);
}

bool World::isAreaBlockedByEntity(const SDL_FRect& rect) const {
    const auto& collisions = m_Collisions.getRaw();
    const auto& entities = m_Collisions.getEntities();

    for (size_t i = 0; i < collisions.size(); i++) {
        const CollisionComponent& collision = collisions[i];
        if (!collision.isBlocking) {
            continue;
        }

        const PositionComponent* position = m_Positions.get(entities[i]);
        if (!position) {
            continue;
        }

        SDL_FRect worldBounds{
            position->position.x + collision.bounds.x,
            position->position.y + collision.bounds.y,
            collision.bounds.w,
            collision.bounds.h
        };

        if (SDL_HasRectIntersectionFloat(&rect, &worldBounds)) {
            return true;
        }
    }

    return false;
}

bool World::satisfiesMachinePlacementCondition(const MachineDefinition& machineDefinition, int tileX, int tileY) const {
    if (machineDefinition.allowedPlacementTags.empty()) {
        return true;
    }

    for (int localY = 0; localY < machineDefinition.heightTiles; localY++) {
        for (int localX = 0; localX < machineDefinition.widthTiles; localX++) {
            const int currentTileX = tileX + localX;
            const int currentTileY = tileY + localY;

            for (size_t layerIndex = 0; layerIndex < m_TileMap.getLayers().size(); layerIndex++) {
                const Tile* tile = m_TileMap.getTile(currentTileX, currentTileY, static_cast<int>(layerIndex));
                if (!tile || !tile->shouldRender || tile->paletteName.empty()) {
                    continue;
                }

                const std::vector<std::string>* surfaceTags =
                    m_TileMetadataDatabase.getSurfaceTags(tile->paletteName, tile->atlasX, tile->atlasY);
                if (!surfaceTags) {
                    continue;
                }

                for (const std::string& surfaceTag : *surfaceTags) {
                    if (std::find(machineDefinition.allowedPlacementTags.begin(),
                                  machineDefinition.allowedPlacementTags.end(),
                                  surfaceTag) != machineDefinition.allowedPlacementTags.end()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool World::canPlaceItem(const ItemDefinition& item, int tileX, int tileY) const {
    if (!item.isPlaceable) {
        return false;
    }

    if (!item.placedMachineUniqueName.empty()) {
        const MachineDefinition* machineDefinition = m_MachineDatabase.getMachine(item.placedMachineUniqueName);
        if (!machineDefinition) {
            return false;
        }

        if (!satisfiesMachinePlacementCondition(*machineDefinition, tileX, tileY)) {
            return false;
        }

        SDL_FRect worldRect{
            static_cast<float>(tileX * 32),
            static_cast<float>(tileY * 32),
            static_cast<float>(machineDefinition->widthTiles * 32),
            static_cast<float>(machineDefinition->heightTiles * 32)
        };

        if (m_TileMap.isRectColliding(worldRect)) {
            return false;
        }

        return !isAreaBlockedByEntity(worldRect);
    }

    if (!item.placeableSprite.texture) {
        return false;
    }

    return m_TileMap.canPlaceTileObject(tileX, tileY, item.placeableLayer, item.placeableWidthTiles, item.placeableHeightTiles);
}

bool World::placeItem(const ItemDefinition& item, int tileX, int tileY) {
    if (!canPlaceItem(item, tileX, tileY)) {
        return false;
    }

    if (!item.placedMachineUniqueName.empty()) {
        return placeMachine(item.placedMachineUniqueName, tileX, tileY);
    }

    return m_TileMap.setTileObject(tileX, tileY, item.placeableSprite, item.placeableLayer, item.placeableWidthTiles, item.placeableHeightTiles, item.placeableBlocking, item.uniqueName);
}

bool World::placeMachine(const std::string& machineUniqueName, int tileX, int tileY) {
    const MachineDefinition* machineDefinition = m_MachineDatabase.getMachine(machineUniqueName);
    if (!machineDefinition) {
        return false;
    }

    if (!satisfiesMachinePlacementCondition(*machineDefinition, tileX, tileY)) {
        return false;
    }

    SDL_FRect worldRect{
        static_cast<float>(tileX * 32),
        static_cast<float>(tileY * 32),
        static_cast<float>(machineDefinition->widthTiles * 32),
        static_cast<float>(machineDefinition->heightTiles * 32)
    };

    if (m_TileMap.isRectColliding(worldRect) || isAreaBlockedByEntity(worldRect)) {
        return false;
    }

    EntityFactory factory(
        m_EntityManager,
        m_Positions,
        m_Velocities,
        m_Inputs,
        m_CharacterStates,
        m_AnimationControllers,
        m_Sprites,
        m_Collisions,
        m_ConveyorBelts,
        m_Inventories,
        m_MachineEntities,
        m_MachineInventories,
        m_CraftingMachines,
        m_Miners,
        m_Interactions,
        m_AnimationLibrary
    );

    const Vec2f worldPosition{static_cast<float>(tileX * 32), static_cast<float>(tileY * 32)};
    const Sprite machineSprite = getMachineSprite(*machineDefinition);

    switch (machineDefinition->type) {
        case MachineType::Crafting:
            return factory.createCraftingMachine(worldPosition, machineSprite, *machineDefinition) != -1;

        case MachineType::Miner: {
            const MinerMachineDefinition* minerDefinition =
                m_MachineDatabase.getMachineAs<MinerMachineDefinition>(machineUniqueName);
            if (!minerDefinition) {
                LOG_WARN("Machine {} is marked as miner, but could not be cast to MinerMachineDefinition", machineUniqueName);
                return false;
            }

            return factory.createMiner(worldPosition, machineSprite, *minerDefinition) != -1;
        }

        default:
            LOG_WARN("Machine {} has unsupported machine type", machineUniqueName);
            return false;
    }
}

void World::clearMachines() {
    std::vector<Entity> machineEntities = m_MachineEntities.getEntities();
    for (Entity entity : machineEntities) {
        m_Positions.remove(entity);
        m_Sprites.remove(entity);
        m_Collisions.remove(entity);
        m_MachineInventories.remove(entity);
        m_CraftingMachines.remove(entity);
        m_Miners.remove(entity);
        m_Interactions.remove(entity);
        m_MachineEntities.remove(entity);
    }
}

std::vector<std::tuple<std::string, int, int>> World::getMachinePlacementData() const {
    std::vector<std::tuple<std::string, int, int>> machines;
    const auto& machineComponents = m_MachineEntities.getRaw();
    const auto& entities = m_MachineEntities.getEntities();

    for (size_t i = 0; i < machineComponents.size(); i++) {
        const PositionComponent* position = m_Positions.get(entities[i]);
        if (!position || machineComponents[i].machineUniqueName.empty()) {
            continue;
        }

        const int tileX = static_cast<int>(position->position.x) / 32;
        const int tileY = static_cast<int>(position->position.y) / 32;
        machines.emplace_back(machineComponents[i].machineUniqueName, tileX, tileY);
    }

    return machines;
}

void World::renderPlacementPreview(const ItemDefinition& item, int tileX, int tileY) const {
    if (!m_Renderer || !item.isPlaceable) {
        return;
    }

    Sprite previewSprite = item.placeableSprite;
    if (!item.placedMachineUniqueName.empty()) {
        const MachineDefinition* machineDefinition = m_MachineDatabase.getMachine(item.placedMachineUniqueName);
        if (!machineDefinition) {
            return;
        }
        previewSprite = getMachineSprite(*machineDefinition);
    }

    if (!previewSprite.texture) {
        return;
    }

    SDL_FRect dest{
        (static_cast<float>(tileX * 32) - m_Camera.getX()) * m_Camera.getZoom(),
        (static_cast<float>(tileY * 32) - m_Camera.getY()) * m_Camera.getZoom(),
        static_cast<float>(item.placeableWidthTiles * 32) * m_Camera.getZoom(),
        static_cast<float>(item.placeableHeightTiles * 32) * m_Camera.getZoom()
    };

    if (!item.placedMachineUniqueName.empty()) {
        const MachineDefinition* machineDefinition = m_MachineDatabase.getMachine(item.placedMachineUniqueName);
        if (!machineDefinition) {
            return;
        }

        dest.w = static_cast<float>(machineDefinition->widthTiles * 32) * m_Camera.getZoom();
        dest.h = static_cast<float>(machineDefinition->heightTiles * 32) * m_Camera.getZoom();
    }

    m_Renderer->drawSpriteAlpha(previewSprite, dest, 160);
    m_Renderer->drawRect(dest, canPlaceItem(item, tileX, tileY) ? SDL_Color{80, 220, 120, 255} : SDL_Color{220, 80, 80, 255});
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
    m_ItemAtlas.createAtlas(m_Renderer, 32, 32, "assets/tilesets/items_sheet.png");
    m_ItemDatabase.loadItemsFromFolder("assets/items", m_ItemAtlas, *m_Renderer);

    LOG_INFO("Loading recipes...");
    m_RecipeDatabase.loadRecipesFromFolder("assets/recipes");

    LOG_INFO("Loading machines...");
    m_MachineDatabase.loadMachinesFromFolder("assets/machines");

    LOG_INFO("Loading tile metadata...");
    m_TileMetadataDatabase.loadFromFolder("assets/tilesets");

    LOG_INFO("Creating entities...");
    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_MachineEntities, m_MachineInventories, m_CraftingMachines, m_Miners, m_Interactions, m_AnimationLibrary);

    m_Player = factory.createPlayer({ 30.0f * 32.0f, 14.0f * 32.0f });
    m_ChunkManager.update({100.0f, 100.0f}, 32);

    auto* inv = m_Inventories.get(m_Player);
    if (inv) {
        inv->inventory.addItem(m_ItemDatabase.getItem("iron_ingot"), 2);
        inv->inventory.addItem(m_ItemDatabase.getItem("copper_ingot"), 4);
        inv->inventory.addItem(m_ItemDatabase.getItem("iron_ingot"), 2);
        inv->inventory.addItem(m_ItemDatabase.getItem("green_gem"), 6);
        inv->inventory.addItem(m_ItemDatabase.getItem("placeable_test"), 3);
        inv->inventory.addItem(m_ItemDatabase.getItem("basic_crafter_item"), 2);
        inv->inventory.addItem(m_ItemDatabase.getItem("basic_miner_item"), 2);
    }

    LOG_INFO("Loading conveyor atlas...");
    m_ConveyorAtlas.createAtlas(m_Renderer, 32, 32, "assets/conveyor_sprites.png");
    m_ConveyorManager.setAtlas(&m_ConveyorAtlas);

    m_OreVeinsAtlas.createAtlas(m_Renderer, 32, 32, "assets/tilesets/ore_veins.png");

    LOG_INFO("Creating map...");
    m_TileMapAtlas.createAtlas(m_Renderer, 16, 16, "assets/Overworld_Tileset.png");
    m_TilePalettes.clear();
    registerTilePalette("Overworld", &m_TileMapAtlas);
    registerTilePalette("Ore Veins", &m_OreVeinsAtlas);
    TileMapSerializer::load(*this, "maps/test.map");

    LOG_INFO("World initialized");
}
