#include "World.hpp"

#include <algorithm>

#include "EntityFactory.hpp"
#include "../Game.hpp"
#include "../Logger.hpp"
#include "../editor/TileMapSerializer.hpp"
#include "../graphics/AnimationLoader.hpp"
#include "../graphics/SpriteAtlas.hpp"

namespace {
    SpriteCoords getStraightConveyorPreviewCoords(Direction direction) {
        switch (direction) {
            case Direction::RIGHT:
                return {0, 5};
            case Direction::DOWN:
                return {0, 6};
            case Direction::LEFT:
                return {7, 5};
            case Direction::UP:
            default:
                return {2, 6};
        }
    }
}

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
                        m_FluidPipes,
                        m_FluidTanks,
                        m_FluidPumps,
                        m_FluidPorts,
                        m_MachineFluids,
                        m_MachineFluidPortLinks,
                        m_MachineEntities,
                        m_MachineInventories,
                        m_CraftingMachines,
                        m_Miners,
                        m_Interactions,
                        m_AnimationLibrary),
      m_FluidManager(m_EntityManager,
                     m_Positions,
                     m_FluidPipes,
                     m_FluidTanks,
                     m_FluidPumps,
                     m_FluidPorts,
                     m_Sprites,
                     m_Velocities,
                     m_Inputs,
                     m_CharacterStates,
                     m_AnimationControllers,
                     m_Collisions,
                     m_ConveyorBelts,
                     m_Inventories,
                     m_MachineFluids,
                     m_MachineFluidPortLinks,
                     m_MachineEntities,
                     m_MachineInventories,
                     m_CraftingMachines,
                     m_Miners,
                     m_Interactions,
                     m_AnimationLibrary,
                     m_FluidSystem) {

}

World::~World() = default;

void World::update(float deltaTime) {
    m_MovementInputSystem.update(m_Inputs, m_Velocities);
    m_CollisionSystem.update(deltaTime, m_Positions, m_Velocities, m_Collisions, m_TileMap);

    CharacterStateSystem::update(m_CharacterStates, m_Velocities);
    AnimationStateSystem::update(m_CharacterStates, m_ConveyorBelts, m_AnimationControllers);
    AnimationSystem::update(deltaTime, m_AnimationControllers);
    m_FluidSystem.update(deltaTime, m_Positions, m_FluidPipes, m_FluidTanks, m_FluidPumps, m_FluidPorts);
    m_MachineFluidIOSystem.update(deltaTime, m_MachineFluidPortLinks, m_FluidPorts, m_CraftingMachines, m_MachineFluids, m_FluidPipes, m_FluidTanks, m_RecipeDatabase, m_FluidDatabase, m_FluidSystem);
    m_CraftingSystem.update(deltaTime, m_CraftingMachines, m_MachineInventories, m_MachineFluids, m_RecipeDatabase, m_ItemDatabase, m_FluidDatabase);
    m_MiningSystem.update(deltaTime, m_Miners, m_Positions, m_MachineInventories, m_TileMap, m_TileMetadataDatabase, m_ItemDatabase);
    m_ConveyorSystem.update(deltaTime, m_EntityManager, m_Player, m_Positions, m_Sprites, m_ConveyorBelts, m_ConveyorItems, m_Inventories, m_MachineInventories);

    if (const auto* playerPos = m_Positions.get(m_Player)) {
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
    AnimatedRenderSystem::render(m_Renderer, m_Camera, m_ChunkManager, m_Positions, m_AnimationControllers);
    if (m_Renderer) {
        m_FluidManager.renderDebug(*m_Renderer, m_Camera, m_ChunkManager);
    }
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
    SpriteAtlas* atlas = getTilePalette(machineDefinition.spritePaletteName);
    if (!atlas) {
        atlas = getTilePalette("Overworld");
    }
    if (!atlas) {
        return {};
    }

    return atlas->getSprite(machineDefinition.spriteAtlasX, machineDefinition.spriteAtlasY);
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

    if (item.placesConveyorBelt) {
        return true;
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

    if (item.placesStorageContainer) {
        SDL_FRect worldRect{
            static_cast<float>(tileX * 32),
            static_cast<float>(tileY * 32),
            static_cast<float>(item.placeableWidthTiles * 32),
            static_cast<float>(item.placeableHeightTiles * 32)
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

bool World::placeItem(const ItemDefinition& item, int tileX, int tileY, Direction direction) {
    if (!canPlaceItem(item, tileX, tileY)) {
        return false;
    }

    if (item.placesConveyorBelt) {
        placeConveyorBelt(tileX, tileY, direction);
        return true;
    }

    if (!item.placedMachineUniqueName.empty()) {
        return placeMachine(item.placedMachineUniqueName, tileX, tileY, direction);
    }

    if (item.placesStorageContainer) {
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
            m_FluidPipes,
            m_FluidTanks,
            m_FluidPumps,
            m_FluidPorts,
            m_MachineFluids,
            m_MachineFluidPortLinks,
            m_MachineEntities,
            m_MachineInventories,
            m_CraftingMachines,
            m_Miners,
            m_Interactions,
            m_AnimationLibrary
        );

        return factory.createStorageContainer(
            {static_cast<float>(tileX * 32), static_cast<float>(tileY * 32)},
            item.placeableSprite,
            {item.containerInventoryWidth, item.containerInventoryHeight},
            {item.placeableWidthTiles, item.placeableHeightTiles},
            item.placeableBlocking
        ) != -1;
    }

    return m_TileMap.setTileObject(tileX, tileY, item.placeableSprite, item.placeableLayer, item.placeableWidthTiles, item.placeableHeightTiles, item.placeableBlocking, item.uniqueName);
}

bool World::placeMachine(const std::string& machineUniqueName, int tileX, int tileY, Direction direction) {
    const MachineDefinition* machineDefinition = m_MachineDatabase.getMachine(machineUniqueName);
    if (!machineDefinition) {
        LOG_WARN("World: rejected machine placement {} at ({}, {}): machine definition missing", machineUniqueName, tileX, tileY);
        return false;
    }

    if (!satisfiesMachinePlacementCondition(*machineDefinition, tileX, tileY)) {
        LOG_WARN("World: rejected machine placement {} at ({}, {}): placement tags not satisfied", machineUniqueName, tileX, tileY);
        return false;
    }

    SDL_FRect worldRect{
        static_cast<float>(tileX * 32),
        static_cast<float>(tileY * 32),
        static_cast<float>(machineDefinition->widthTiles * 32),
        static_cast<float>(machineDefinition->heightTiles * 32)
    };

    if (m_TileMap.isRectColliding(worldRect) || isAreaBlockedByEntity(worldRect)) {
        LOG_WARN("World: rejected machine placement {} at ({}, {}): blocked or colliding", machineUniqueName, tileX, tileY);
        return false;
    }

    LOG_INFO("World: placing machine {} at ({}, {}) direction={}", machineUniqueName, tileX, tileY, static_cast<int>(direction));

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
        m_FluidPipes,
        m_FluidTanks,
        m_FluidPumps,
        m_FluidPorts,
        m_MachineFluids,
        m_MachineFluidPortLinks,
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
        case MachineType::Crafting: {
            const Entity entity = factory.createCraftingMachine(worldPosition, machineSprite, *machineDefinition);
            if (!machineDefinition->fluidPorts.empty()) {
                m_FluidSystem.markNetworksDirty();
            }
            if (entity != -1) {
                LOG_INFO("World: placed crafting machine {} entity={}", machineUniqueName, entity);
            }
            return entity != -1;
        }

        case MachineType::Miner: {
            const MinerMachineDefinition* minerDefinition =
                m_MachineDatabase.getMachineAs<MinerMachineDefinition>(machineUniqueName);
            if (!minerDefinition) {
                LOG_WARN("Machine {} is marked as miner, but could not be cast to MinerMachineDefinition", machineUniqueName);
                return false;
            }

            const Entity entity = factory.createMiner(worldPosition, machineSprite, *minerDefinition);
            if (entity != -1) {
                LOG_INFO("World: placed miner {} entity={}", machineUniqueName, entity);
            }
            return entity != -1;
        }

        case MachineType::FluidTank: {
            const FluidTankMachineDefinition* tankDefinition =
                m_MachineDatabase.getMachineAs<FluidTankMachineDefinition>(machineUniqueName);
            if (!tankDefinition) {
                LOG_WARN("Machine {} is marked as fluid tank, but could not be cast to FluidTankMachineDefinition", machineUniqueName);
                return false;
            }

            const Entity entity = factory.createFluidTankMachine(worldPosition, machineSprite, *tankDefinition);
            m_FluidManager.registerFluidTankEntity(tileX, tileY, entity);
            m_FluidSystem.markNetworksDirty();
            if (entity != -1) {
                LOG_INFO("World: placed fluid tank {} entity={}", machineUniqueName, entity);
            }
            return entity != -1;
        }

        case MachineType::FluidPump: {
            const FluidPumpMachineDefinition* pumpDefinition =
                m_MachineDatabase.getMachineAs<FluidPumpMachineDefinition>(machineUniqueName);
            if (!pumpDefinition) {
                LOG_WARN("Machine {} is marked as fluid pump, but could not be cast to FluidPumpMachineDefinition", machineUniqueName);
                return false;
            }

            const Entity entity = factory.createFluidPumpMachine(worldPosition, machineSprite, *pumpDefinition, direction, m_FluidManager.getDebugFluidDefinition());
            m_FluidManager.registerFluidPumpEntity(tileX, tileY, entity);
            m_FluidSystem.markNetworksDirty();
            if (entity != -1) {
                LOG_INFO("World: placed fluid pump {} entity={} direction={}", machineUniqueName, entity, static_cast<int>(direction));
            }
            return entity != -1;
        }

        default:
            LOG_WARN("Machine {} has unsupported machine type", machineUniqueName);
            return false;
    }
}

void World::clearMachines() {
    std::vector<Entity> machineEntities = m_MachineEntities.getEntities();
    bool removedFluidMachine = false;
    for (Entity entity : machineEntities) {
        const PositionComponent* position = m_Positions.get(entity);
        const int tileX = position ? static_cast<int>(position->position.x) / 32 : 0;
        const int tileY = position ? static_cast<int>(position->position.y) / 32 : 0;

        if (m_FluidTanks.get(entity)) {
            m_FluidManager.unregisterFluidTankEntity(tileX, tileY);
            m_FluidTanks.remove(entity);
            removedFluidMachine = true;
        }

        if (m_FluidPumps.get(entity)) {
            m_FluidManager.unregisterFluidPumpEntity(tileX, tileY);
            m_FluidPumps.remove(entity);
            m_FluidPorts.remove(entity);
            removedFluidMachine = true;
        }

        if (m_MachineFluids.get(entity)) {
            removedFluidMachine = true;
        }

        std::vector<Entity> machinePortEntities;
        const auto& portLinks = m_MachineFluidPortLinks.getRaw();
        const auto& portLinkEntities = m_MachineFluidPortLinks.getEntities();
        for (size_t i = 0; i < portLinks.size(); i++) {
            if (portLinks[i].machineEntity == entity) {
                machinePortEntities.push_back(portLinkEntities[i]);
            }
        }

        for (Entity portEntity : machinePortEntities) {
            m_Positions.remove(portEntity);
            m_FluidPorts.remove(portEntity);
            m_MachineFluidPortLinks.remove(portEntity);
        }

        m_Positions.remove(entity);
        m_Sprites.remove(entity);
        m_Collisions.remove(entity);
        m_MachineFluids.remove(entity);
        m_MachineInventories.remove(entity);
        m_CraftingMachines.remove(entity);
        m_Miners.remove(entity);
        m_Interactions.remove(entity);
        m_MachineEntities.remove(entity);
    }

    if (removedFluidMachine) {
        m_FluidSystem.markNetworksDirty();
    }
}

std::vector<std::tuple<std::string, int, int, Direction>> World::getMachinePlacementData() const {
    std::vector<std::tuple<std::string, int, int, Direction>> machines;
    const auto& machineComponents = m_MachineEntities.getRaw();
    const auto& entities = m_MachineEntities.getEntities();

    for (size_t i = 0; i < machineComponents.size(); i++) {
        const PositionComponent* position = m_Positions.get(entities[i]);
        if (!position || machineComponents[i].machineUniqueName.empty()) {
            continue;
        }

        const int tileX = static_cast<int>(position->position.x) / 32;
        const int tileY = static_cast<int>(position->position.y) / 32;
        Direction direction = Direction::RIGHT;
        if (const FluidPortComponent* directPort = m_FluidPorts.get(entities[i])) {
            direction = directPort->side;
        }
        const auto& portLinks = m_MachineFluidPortLinks.getRaw();
        const auto& portLinkEntities = m_MachineFluidPortLinks.getEntities();
        for (size_t portIndex = 0; portIndex < portLinks.size(); portIndex++) {
            if (portLinks[portIndex].machineEntity != entities[i]) {
                continue;
            }

            if (const FluidPortComponent* port = m_FluidPorts.get(portLinkEntities[portIndex])) {
                direction = port->side;
                break;
            }
        }
        machines.emplace_back(machineComponents[i].machineUniqueName, tileX, tileY, direction);
    }

    return machines;
}

void World::renderPlacementPreview(const ItemDefinition& item, int tileX, int tileY, Direction direction) const {
    if (!m_Renderer || !item.isPlaceable) {
        return;
    }

    Sprite previewSprite = item.placeableSprite;
    if (item.placesConveyorBelt) {
        const SpriteCoords coords = getStraightConveyorPreviewCoords(direction);
        previewSprite = m_ConveyorAtlas.getSprite(coords.x, coords.y);
    }

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

void World::placeFluidPipe(int tileX, int tileY, Direction direction) {
    m_FluidManager.placeFluidPipe(tileX, tileY, direction);
}

void World::removeFluidPipe(int tileX, int tileY) {
    m_FluidManager.removeFluidPipe(tileX, tileY);
}

void World::placeFluidTank(int tileX, int tileY) {
    placeMachine("fluid_tank", tileX, tileY);
}

void World::removeFluidTank(int tileX, int tileY) {
    m_FluidManager.removeFluidTank(tileX, tileY);
}

void World::placeFluidPump(int tileX, int tileY, Direction direction) {
    placeMachine("fluid_pump", tileX, tileY, direction);
}

void World::removeFluidPump(int tileX, int tileY) {
    m_FluidManager.removeFluidPump(tileX, tileY);
}

bool World::addDebugFluidToTank(int tileX, int tileY, float amount) {
    return m_FluidManager.addDebugFluidToTank(tileX, tileY, amount);
}

void World::clearConveyorBelts() {
    m_ConveyorManager.clearConveyorBelts();
}

std::vector<std::tuple<int, int, Direction>> World::getConveyorBeltData() const {
    return m_ConveyorManager.getConveyorBeltData();
}

Entity World::getHoveredMachine(float worldX, float worldY) const {
    if (!m_ChunkManager.isWorldPositionLoaded(worldX, worldY, 32))
        return 0;
    const auto& entities = m_MachineEntities.getEntities();
    for (Entity entity : entities) {
        const auto* position = m_Positions.get(entity);
        const auto* collisionComp = m_Collisions.get(entity);
        if (!position || !collisionComp) {
            continue;
        }
        SDL_FRect rect { position->position.x, position->position.y, collisionComp->bounds.w, collisionComp->bounds.h };
        if (worldX >= rect.x && worldX <= rect.x + rect.w && worldY >= rect.y && worldY <= rect.y + rect.h) {
            return entity;
        }
    }
    return 0;
}

void World::renderMachineHighlight(Entity machine) const {
    if (machine == 0) return;

    const auto* position = m_Positions.get(machine);
    const auto* collisionComp = m_Collisions.get(machine);
    if (!position || !collisionComp) {
        return;
    }
    SDL_FRect worldPos{position->position.x, position->position.y, collisionComp->bounds.w, collisionComp->bounds.h};
    SDL_FRect screenPos{(worldPos.x - m_Camera.getX()) * m_Camera.getZoom(), (worldPos.y - m_Camera.getY()) * m_Camera.getZoom(), collisionComp->bounds.w * m_Camera.getZoom(), collisionComp->bounds.h * m_Camera.getZoom()};
    m_Renderer->drawRect(screenPos, SDL_Color{255, 255, 0, 255});
}

void World::initializeWorld() {

    LOG_INFO("Loading animations...");
    AnimationLoader::createSpriteAtlas(m_Renderer);
    AnimationLoader::loadAnimations(m_Renderer, m_AnimationLibrary);

    LOG_INFO("Loading items...");
    m_ItemAtlas.createAtlas(m_Renderer, 32, 32, "assets/tilesets/items_sheet.png");
    m_ItemDatabase.loadItemsFromFolder("assets/items", m_ItemAtlas, *m_Renderer);

    LOG_INFO("Loading fluids...");
    m_FluidDatabase.loadFluidsFromFolder("assets/fluids");
    const FluidDefinition* defaultFluid = m_FluidDatabase.getFluid("water");
    m_FluidManager.setDefaultFluidDefinition(defaultFluid);
    if (!defaultFluid) {
        LOG_WARN("World: default fluid 'water' not found in FluidDatabase, using local fallback");
    }

    LOG_INFO("Loading recipes...");
    m_RecipeDatabase.loadRecipesFromFolder("assets/recipes");

    LOG_INFO("Loading machines...");
    m_MachineDatabase.loadMachinesFromFolder("assets/machines");

    LOG_INFO("Loading tile metadata...");
    m_TileMetadataDatabase.loadFromFolder("assets/tilesets");

    LOG_INFO("Creating entities...");
    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_FluidPipes, m_FluidTanks, m_FluidPumps, m_FluidPorts, m_MachineFluids, m_MachineFluidPortLinks, m_MachineEntities, m_MachineInventories, m_CraftingMachines, m_Miners, m_Interactions, m_AnimationLibrary);

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
        inv->inventory.addItem(m_ItemDatabase.getItem("conveyer_item"), 20);
        inv->inventory.addItem(m_ItemDatabase.getItem("test_container_item"), 2);
        inv->inventory.addItem(m_ItemDatabase.getItem("steel_ingot"), 3);
    }

    LOG_INFO("Loading conveyor atlas...");
    m_ConveyorAtlas.createAtlas(m_Renderer, 32, 32, "assets/conveyor_sprites.png");
    m_ConveyorManager.setAtlas(&m_ConveyorAtlas);

    LOG_INFO("Loading fluid atlas...");
    m_FluidAtlas.createAtlas(m_Renderer, 32, 32, "assets/pipes.png");
    m_FluidManager.setAtlas(&m_FluidAtlas);

    m_OreVeinsAtlas.createAtlas(m_Renderer, 32, 32, "assets/tilesets/ore_veins.png");

    LOG_INFO("Creating map...");
    m_TileMapAtlas.createAtlas(m_Renderer, 32, 32, "assets/tilesets/Map_tiles.png");
    m_TilePalettes.clear();
    registerTilePalette("Overworld", &m_TileMapAtlas);
    registerTilePalette("Fluid", &m_FluidAtlas);
    registerTilePalette("Ore Veins", &m_OreVeinsAtlas);
    TileMapSerializer::load(*this, "maps/test.map");
    // TileMapGenerator::Config mapGeneratorConfig;
    // TileMapGenerator::generateTerrain(m_TileMap, m_TileMapAtlas, mapGeneratorConfig);

    LOG_INFO("World initialized");
}
