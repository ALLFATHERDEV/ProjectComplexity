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

bool World::canPlaceItem(const ItemDefinition& item, int tileX, int tileY) const {
    if (!item.isPlaceable) {
        return false;
    }

    if (!item.placedMachineUniqueName.empty()) {
        const MachineDefinition* machineDefinition = m_MachineDatabase.getMachine(item.placedMachineUniqueName);
        if (!machineDefinition) {
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
        const MachineDefinition* machineDefinition = m_MachineDatabase.getMachine(item.placedMachineUniqueName);
        if (!machineDefinition) {
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
            m_MachineInventories,
            m_CraftingMachines,
            m_Interactions,
            m_AnimationLibrary
        );

        factory.createCraftingMachine(
            {static_cast<float>(tileX * 32), static_cast<float>(tileY * 32)},
            getMachineSprite(*machineDefinition),
            *machineDefinition
        );
        return true;
    }

    return m_TileMap.setTileObject(tileX, tileY, item.placeableSprite, item.placeableLayer, item.placeableWidthTiles, item.placeableHeightTiles, item.placeableBlocking, item.uniqueName);
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
    m_ItemAtlas.createAtlas(m_Renderer, 32, 32, "assets/spritesheets/items_sheet.png");
    m_ItemDatabase.loadItemsFromFolder("assets/items", m_ItemAtlas, *m_Renderer);

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
        inv->inventory.addItem(m_ItemDatabase.getItem("green_gem"), 6);
        inv->inventory.addItem(m_ItemDatabase.getItem("placeable_test"), 3);
        inv->inventory.addItem(m_ItemDatabase.getItem("basic_crafter_item"), 2);
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

    m_Machine = factory.createCraftingMachine({ 100.0f, 100.0f }, getMachineSprite(*basicCrafter), *basicCrafter);

    const ItemDefinition* iron = m_ItemDatabase.getItem("iron_ingot");
    auto* machineInventory = m_MachineInventories.get(m_Machine);
    if (iron && machineInventory) {
        machineInventory->inputInventory.addItem(iron, 4);
    }

    LOG_INFO("World initialized");
}
