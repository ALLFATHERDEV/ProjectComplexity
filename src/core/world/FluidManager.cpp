#include "FluidManager.hpp"

#include <algorithm>

#include "ChunkManager.hpp"
#include "EntityFactory.hpp"



constexpr PipeSpriteLayout kFluidSprites{};

FluidManager::FluidManager(EntityManager& entityManager,
                           ComponentStorage<PositionComponent>& positions,
                           ComponentStorage<FluidPipeComponent>& fluidPipes,
                           ComponentStorage<FluidTankComponent>& fluidTanks,
                           ComponentStorage<FluidPumpComponent>& fluidPumps,
                           ComponentStorage<FluidPortComponent>& fluidPorts,
                           ComponentStorage<SpriteComponent>& sprites,
                           ComponentStorage<VelocityComponent>& velocities,
                           ComponentStorage<InputComponent>& inputs,
                           ComponentStorage<CharacterStateComponent>& characterStates,
                           ComponentStorage<AnimationControllerComponent>& animationControllers,
                           ComponentStorage<CollisionComponent>& collisions,
                           ComponentStorage<ConveyorBeltComponent>& conveyorBelts,
                           ComponentStorage<InventoryComponent>& inventories,
                           ComponentStorage<MachineFluidComponent>& machineFluids,
                           ComponentStorage<MachineFluidPortLinkComponent>& machineFluidPortLinks,
                           ComponentStorage<MachineComponent>& machines,
                           ComponentStorage<MachineInventoryComponent>& machineInventories,
                           ComponentStorage<CraftingMachineComponent>& craftingMachines,
                           ComponentStorage<MinerComponent>& miners,
                           ComponentStorage<InteractionComponent>& interactions,
                           AnimationLibrary& animationLibrary,
                           FluidSystem& fluidSystem)
    : m_EntityManager(entityManager),
      m_Positions(positions),
      m_FluidPipes(fluidPipes),
      m_FluidTanks(fluidTanks),
      m_FluidPumps(fluidPumps),
      m_FluidPorts(fluidPorts),
      m_Sprites(sprites),
      m_Velocities(velocities),
      m_Inputs(inputs),
      m_CharacterStates(characterStates),
      m_AnimationControllers(animationControllers),
      m_Collisions(collisions),
      m_ConveyorBelts(conveyorBelts),
      m_Inventories(inventories),
      m_MachineFluids(machineFluids),
      m_MachineFluidPortLinks(machineFluidPortLinks),
      m_Machines(machines),
      m_MachineInventories(machineInventories),
      m_CraftingMachines(craftingMachines),
      m_Miners(miners),
      m_Interactions(interactions),
      m_AnimationLibrary(animationLibrary),
      m_FluidSystem(fluidSystem) {
}

void FluidManager::setAtlas(SpriteAtlas* atlas) {
    m_FluidAtlas = atlas;
}

void FluidManager::registerFluidTankEntity(int tileX, int tileY, Entity entity) {
    m_FluidTankEntitiesByTile[makeTileKey(tileX, tileY)] = entity;
    refreshFluidSpritesAround(tileX, tileY);
}

void FluidManager::registerFluidPumpEntity(int tileX, int tileY, Entity entity) {
    m_FluidPumpEntitiesByTile[makeTileKey(tileX, tileY)] = entity;
    refreshFluidSpritesAround(tileX, tileY);
}

void FluidManager::unregisterFluidTankEntity(int tileX, int tileY) {
    m_FluidTankEntitiesByTile.erase(makeTileKey(tileX, tileY));
    refreshFluidSpritesAround(tileX, tileY);
}

void FluidManager::unregisterFluidPumpEntity(int tileX, int tileY) {
    m_FluidPumpEntitiesByTile.erase(makeTileKey(tileX, tileY));
    refreshFluidSpritesAround(tileX, tileY);
}

void FluidManager::setDefaultFluidDefinition(const FluidDefinition* fluidDefinition) {
    m_DefaultFluidDefinition = fluidDefinition;
}

const FluidDefinition* FluidManager::getDebugFluidDefinition() const {
    return m_DefaultFluidDefinition ? m_DefaultFluidDefinition : &m_DebugWater;
}

long long FluidManager::makeTileKey(int tileX, int tileY) {
    const unsigned long long x = static_cast<unsigned int>(tileX);
    const unsigned long long y = static_cast<unsigned int>(tileY);
    return static_cast<long long>((x << 32) | y);
}

bool FluidManager::hasConnectableNeighbor(int tileX, int tileY, Direction direction) const {
    int neighborTileX = tileX;
    int neighborTileY = tileY;

    switch (direction) {
        case Direction::UP:
            neighborTileY -= 1;
            break;
        case Direction::DOWN:
            neighborTileY += 1;
            break;
        case Direction::LEFT:
            neighborTileX -= 1;
            break;
        case Direction::RIGHT:
            neighborTileX += 1;
            break;
    }

    if (getFluidPipeAtTile(neighborTileX, neighborTileY) != 0) {
        return true;
    }

    if (getFluidTankAtTile(neighborTileX, neighborTileY) != 0) {
        return true;
    }

    return hasCompatiblePortAtTile(neighborTileX, neighborTileY, direction);
}

bool FluidManager::hasCompatiblePortAtTile(int tileX, int tileY, Direction direction) const {
    const auto& portEntities = m_FluidPorts.getEntities();
    for (Entity entity : portEntities) {
        const PositionComponent* position = m_Positions.get(entity);
        const FluidPortComponent* port = m_FluidPorts.get(entity);
        if (!position || !port) {
            continue;
        }

        const int entityTileX = static_cast<int>(position->position.x) / static_cast<int>(kTileSize);
        const int entityTileY = static_cast<int>(position->position.y) / static_cast<int>(kTileSize);
        if (entityTileX != tileX || entityTileY != tileY) {
            continue;
        }

        switch (direction) {
            case Direction::UP:
                if (port->side == Direction::DOWN) {
                    return true;
                }
                break;
            case Direction::DOWN:
                if (port->side == Direction::UP) {
                    return true;
                }
                break;
            case Direction::LEFT:
                if (port->side == Direction::RIGHT) {
                    return true;
                }
                break;
            case Direction::RIGHT:
                if (port->side == Direction::LEFT) {
                    return true;
                }
                break;
        }
    }

    return false;
}

Sprite FluidManager::getPipeSprite(const FluidPipeComponent& pipe) const {
    if (!m_FluidAtlas) {
        return {nullptr, {0.0f, 0.0f, 0.0f, 0.0f}};
    }

    int mask = 0;
    if (pipe.connectUp) {
        mask |= 1;
    }
    if (pipe.connectRight) {
        mask |= 2;
    }
    if (pipe.connectDown) {
        mask |= 4;
    }
    if (pipe.connectLeft) {
        mask |= 8;
    }

    switch (mask) {
        case 0:
            if (pipe.direction == Direction::UP || pipe.direction == Direction::DOWN) {
                return m_FluidAtlas->getSprite(kFluidSprites.isolatedVertical.x, kFluidSprites.isolatedVertical.y);
            }
            return m_FluidAtlas->getSprite(kFluidSprites.isolatedHorizontal.x, kFluidSprites.isolatedHorizontal.y);
        case 1:
            return m_FluidAtlas->getSprite(kFluidSprites.endUp.x, kFluidSprites.endUp.y);
        case 2:
            return m_FluidAtlas->getSprite(kFluidSprites.endRight.x, kFluidSprites.endRight.y);
        case 4:
            return m_FluidAtlas->getSprite(kFluidSprites.endDown.x, kFluidSprites.endDown.y);
        case 8:
            return m_FluidAtlas->getSprite(kFluidSprites.endLeft.x, kFluidSprites.endLeft.y);
        case 1 | 4:
            return m_FluidAtlas->getSprite(kFluidSprites.vertical.x, kFluidSprites.vertical.y);
        case 2 | 8:
            return m_FluidAtlas->getSprite(kFluidSprites.horizontal.x, kFluidSprites.horizontal.y);
        case 1 | 2:
            return m_FluidAtlas->getSprite(kFluidSprites.cornerUpRight.x, kFluidSprites.cornerUpRight.y);
        case 2 | 4:
            return m_FluidAtlas->getSprite(kFluidSprites.cornerRightDown.x, kFluidSprites.cornerRightDown.y);
        case 4 | 8:
            return m_FluidAtlas->getSprite(kFluidSprites.cornerDownLeft.x, kFluidSprites.cornerDownLeft.y);
        case 8 | 1:
            return m_FluidAtlas->getSprite(kFluidSprites.cornerLeftUp.x, kFluidSprites.cornerLeftUp.y);
        case 1 | 2 | 4:
            return m_FluidAtlas->getSprite(kFluidSprites.teeNoLeft.x, kFluidSprites.teeNoLeft.y);
        case 2 | 4 | 8:
            return m_FluidAtlas->getSprite(kFluidSprites.teeNoUp.x, kFluidSprites.teeNoUp.y);
        case 4 | 8 | 1:
            return m_FluidAtlas->getSprite(kFluidSprites.teeNoRight.x, kFluidSprites.teeNoRight.y);
        case 8 | 1 | 2:
            return m_FluidAtlas->getSprite(kFluidSprites.teeNoDown.x, kFluidSprites.teeNoDown.y);
        case 1 | 2 | 4 | 8:
            return m_FluidAtlas->getSprite(kFluidSprites.cross.x, kFluidSprites.cross.y);
        default:
            return m_FluidAtlas->getSprite(kFluidSprites.horizontal.x, kFluidSprites.horizontal.y);
    }
}

Sprite FluidManager::getTankSprite() const {
    if (!m_FluidAtlas) {
        return {nullptr, {0.0f, 0.0f, 0.0f, 0.0f}};
    }

    return m_FluidAtlas->getSprite(kFluidSprites.tank.x, kFluidSprites.tank.y);
}

Sprite FluidManager::getPumpSprite(Direction direction) const {
    if (!m_FluidAtlas) {
        return {nullptr, {0.0f, 0.0f, 0.0f, 0.0f}};
    }

    switch (direction) {
        case Direction::UP:
            return m_FluidAtlas->getSprite(kFluidSprites.pumpUp.x, kFluidSprites.pumpUp.y);
        case Direction::RIGHT:
            return m_FluidAtlas->getSprite(kFluidSprites.pumpRight.x, kFluidSprites.pumpRight.y);
        case Direction::DOWN:
            return m_FluidAtlas->getSprite(kFluidSprites.pumpDown.x, kFluidSprites.pumpDown.y);
        case Direction::LEFT:
            return m_FluidAtlas->getSprite(kFluidSprites.pumpLeft.x, kFluidSprites.pumpLeft.y);
        default:
            return m_FluidAtlas->getSprite(kFluidSprites.pumpRight.x, kFluidSprites.pumpRight.y);
    }
}

void FluidManager::refreshFluidSpriteAt(int tileX, int tileY) {
    if (const Entity pipe = getFluidPipeAtTile(tileX, tileY); pipe != 0) {
        FluidPipeComponent* pipeComponent = m_FluidPipes.get(pipe);
        SpriteComponent* sprite = m_Sprites.get(pipe);
        if (pipeComponent && sprite) {
            const bool hasVerticalOrientation = pipeComponent->direction == Direction::UP || pipeComponent->direction == Direction::DOWN;
            const bool connectUp = hasConnectableNeighbor(tileX, tileY, Direction::UP);
            const bool connectRight = hasConnectableNeighbor(tileX, tileY, Direction::RIGHT);
            const bool connectDown = hasConnectableNeighbor(tileX, tileY, Direction::DOWN);
            const bool connectLeft = hasConnectableNeighbor(tileX, tileY, Direction::LEFT);

            const bool hasNeighbors = connectUp || connectRight || connectDown || connectLeft;
            pipeComponent->connectUp = connectUp;
            pipeComponent->connectRight = connectRight;
            pipeComponent->connectDown = connectDown;
            pipeComponent->connectLeft = connectLeft;

            if (!hasNeighbors) {
                pipeComponent->connectUp = hasVerticalOrientation;
                pipeComponent->connectDown = hasVerticalOrientation;
                pipeComponent->connectLeft = !hasVerticalOrientation;
                pipeComponent->connectRight = !hasVerticalOrientation;
            }

            sprite->sprite = getPipeSprite(*pipeComponent);
            sprite->renderWidth = kTileSize;
            sprite->renderHeight = kTileSize;
            sprite->visible = true;
        }
        return;
    }

    if (const Entity tank = getFluidTankAtTile(tileX, tileY); tank != 0) {
        if (SpriteComponent* sprite = m_Sprites.get(tank)) {
            sprite->sprite = getTankSprite();
            sprite->renderWidth = kTileSize;
            sprite->renderHeight = kTileSize;
            sprite->visible = true;
        }
        return;
    }

    if (const Entity pump = getFluidPumpAtTile(tileX, tileY); pump != 0) {
        if (SpriteComponent* sprite = m_Sprites.get(pump)) {
            Direction direction = Direction::RIGHT;
            if (const FluidPortComponent* port = m_FluidPorts.get(pump)) {
                direction = port->side;
            }
            sprite->sprite = getPumpSprite(direction);
            sprite->renderWidth = kTileSize;
            sprite->renderHeight = kTileSize;
            sprite->visible = true;
        }
    }
}

void FluidManager::refreshFluidSpritesAround(int tileX, int tileY) {
    refreshFluidSpriteAt(tileX, tileY);
    refreshFluidSpriteAt(tileX + 1, tileY);
    refreshFluidSpriteAt(tileX - 1, tileY);
    refreshFluidSpriteAt(tileX, tileY + 1);
    refreshFluidSpriteAt(tileX, tileY - 1);
}

void FluidManager::placeFluidPipe(int tileX, int tileY, Direction direction) {
    if (!m_FluidAtlas) {
        return;
    }

    if (getFluidPipeAtTile(tileX, tileY) != 0 || getFluidTankAtTile(tileX, tileY) != 0 || getFluidPumpAtTile(tileX, tileY) != 0) {
        return;
    }

    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_FluidPipes, m_FluidTanks, m_FluidPumps, m_FluidPorts, m_MachineFluids, m_MachineFluidPortLinks, m_Machines, m_MachineInventories, m_CraftingMachines, m_Miners, m_Interactions, m_AnimationLibrary);

    FluidPipeComponent previewPipe;
    previewPipe.direction = direction;
    previewPipe.connectUp = direction == Direction::UP || direction == Direction::DOWN;
    previewPipe.connectDown = previewPipe.connectUp;
    previewPipe.connectLeft = !previewPipe.connectUp;
    previewPipe.connectRight = previewPipe.connectLeft;

    const Entity pipe = factory.createFluidPipe(
        {static_cast<float>(tileX) * kTileSize, static_cast<float>(tileY) * kTileSize},
        getPipeSprite(previewPipe),
        direction
    );
    m_FluidPipeEntitiesByTile[makeTileKey(tileX, tileY)] = pipe;
    refreshFluidSpritesAround(tileX, tileY);
    m_FluidSystem.markNetworksDirty();
}

void FluidManager::removeFluidPipe(int tileX, int tileY) {
    const long long key = makeTileKey(tileX, tileY);
    auto it = m_FluidPipeEntitiesByTile.find(key);
    if (it == m_FluidPipeEntitiesByTile.end()) {
        return;
    }

    const Entity pipe = it->second;
    m_Positions.remove(pipe);
    m_Sprites.remove(pipe);
    m_FluidPipes.remove(pipe);
    m_FluidPipeEntitiesByTile.erase(it);
    refreshFluidSpritesAround(tileX, tileY);
    m_FluidSystem.markNetworksDirty();
}

void FluidManager::placeFluidTank(int tileX, int tileY) {
    if (!m_FluidAtlas) {
        return;
    }

    if (getFluidPipeAtTile(tileX, tileY) != 0 || getFluidTankAtTile(tileX, tileY) != 0 || getFluidPumpAtTile(tileX, tileY) != 0) {
        return;
    }

    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_FluidPipes, m_FluidTanks, m_FluidPumps, m_FluidPorts, m_MachineFluids, m_MachineFluidPortLinks, m_Machines, m_MachineInventories, m_CraftingMachines, m_Miners, m_Interactions, m_AnimationLibrary);

    const Entity tank = factory.createFluidTank(
        {static_cast<float>(tileX) * kTileSize, static_cast<float>(tileY) * kTileSize},
        getTankSprite()
    );
    m_FluidTankEntitiesByTile[makeTileKey(tileX, tileY)] = tank;
    refreshFluidSpritesAround(tileX, tileY);
    m_FluidSystem.markNetworksDirty();
}

void FluidManager::removeFluidTank(int tileX, int tileY) {
    const long long key = makeTileKey(tileX, tileY);
    auto it = m_FluidTankEntitiesByTile.find(key);
    if (it == m_FluidTankEntitiesByTile.end()) {
        return;
    }

    const Entity tank = it->second;
    m_Positions.remove(tank);
    m_Sprites.remove(tank);
    m_Collisions.remove(tank);
    m_FluidTanks.remove(tank);
    m_Interactions.remove(tank);
    m_Machines.remove(tank);
    m_FluidTankEntitiesByTile.erase(it);
    refreshFluidSpritesAround(tileX, tileY);
    m_FluidSystem.markNetworksDirty();
}

void FluidManager::placeFluidPump(int tileX, int tileY, Direction direction) {
    if (!m_FluidAtlas) {
        return;
    }

    if (getFluidPipeAtTile(tileX, tileY) != 0 || getFluidTankAtTile(tileX, tileY) != 0 || getFluidPumpAtTile(tileX, tileY) != 0) {
        return;
    }

    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_FluidPipes, m_FluidTanks, m_FluidPumps, m_FluidPorts, m_MachineFluids, m_MachineFluidPortLinks, m_Machines, m_MachineInventories, m_CraftingMachines, m_Miners, m_Interactions, m_AnimationLibrary);

    const Entity pump = factory.createFluidPump(
        {static_cast<float>(tileX) * kTileSize, static_cast<float>(tileY) * kTileSize},
        getPumpSprite(direction),
        direction,
        &m_DebugWater,
        100.0f
    );
    m_FluidPumpEntitiesByTile[makeTileKey(tileX, tileY)] = pump;
    refreshFluidSpritesAround(tileX, tileY);
    m_FluidSystem.markNetworksDirty();
}

void FluidManager::removeFluidPump(int tileX, int tileY) {
    const long long key = makeTileKey(tileX, tileY);
    auto it = m_FluidPumpEntitiesByTile.find(key);
    if (it == m_FluidPumpEntitiesByTile.end()) {
        return;
    }

    const Entity pump = it->second;
    m_Positions.remove(pump);
    m_Sprites.remove(pump);
    m_Collisions.remove(pump);
    m_FluidPumps.remove(pump);
    m_FluidPorts.remove(pump);
    m_Interactions.remove(pump);
    m_Machines.remove(pump);
    m_FluidPumpEntitiesByTile.erase(it);
    refreshFluidSpritesAround(tileX, tileY);
    m_FluidSystem.markNetworksDirty();
}

bool FluidManager::addDebugFluidToTank(int tileX, int tileY, float amount) {
    const Entity tank = getFluidTankAtTile(tileX, tileY);
    if (tank == 0 || amount <= 0.0f) {
        return false;
    }

    FluidTankComponent* tankComponent = m_FluidTanks.get(tank);
    if (!tankComponent) {
        return false;
    }

    const FluidDefinition* debugFluid = getDebugFluidDefinition();
    if (!debugFluid) {
        return false;
    }

    if (tankComponent->storage.fluid && tankComponent->storage.fluid != debugFluid) {
        return false;
    }

    tankComponent->storage.fluid = debugFluid;
    tankComponent->storage.amount = std::min(tankComponent->capacity, tankComponent->storage.amount + amount);
    return true;
}

void FluidManager::renderDebug(Renderer& renderer, const Camera2D& camera, const ChunkManager& chunkManager) const {
    const float zoom = camera.getZoom();

    const std::vector<Entity>& pipeEntities = m_FluidPipes.getEntities();
    for (Entity entity : pipeEntities) {
        const PositionComponent* position = m_Positions.get(entity);
        const FluidPipeComponent* pipe = m_FluidPipes.get(entity);
        if (!position || !pipe || !chunkManager.isWorldPositionLoaded(position->position.x, position->position.y, 32)) {
            continue;
        }

        if (pipe->capacity <= 0.0f || pipe->storage.amount <= 0.0f) {
            continue;
        }

        SDL_FRect rect{
            (position->position.x - camera.getX()) * zoom,
            (position->position.y - camera.getY()) * zoom,
            kTileSize * zoom,
            kTileSize * zoom
        };

        const float fillRatio = std::clamp(pipe->storage.amount / pipe->capacity, 0.0f, 1.0f);
        SDL_FRect fillRect{
            rect.x + rect.w * 0.15f,
            rect.y + rect.h * (1.0f - fillRatio),
            rect.w * 0.7f,
            rect.h * fillRatio
        };
        renderer.drawFilledRect(fillRect, SDL_Color{90, 180, 255, 220});
    }

    const std::vector<Entity>& tankEntities = m_FluidTanks.getEntities();
    for (Entity entity : tankEntities) {
        const PositionComponent* position = m_Positions.get(entity);
        const FluidTankComponent* tank = m_FluidTanks.get(entity);
        if (!position || !tank || !chunkManager.isWorldPositionLoaded(position->position.x, position->position.y, 32)) {
            continue;
        }

        if (tank->capacity <= 0.0f || tank->storage.amount <= 0.0f) {
            continue;
        }

        SDL_FRect rect{
            (position->position.x - camera.getX()) * zoom,
            (position->position.y - camera.getY()) * zoom,
            kTileSize * zoom,
            kTileSize * zoom
        };

        const float fillRatio = std::clamp(tank->storage.amount / tank->capacity, 0.0f, 1.0f);
        SDL_FRect fillRect{
            rect.x + rect.w * 0.15f,
            rect.y + rect.h * (1.0f - fillRatio),
            rect.w * 0.7f,
            rect.h * fillRatio
        };
        renderer.drawFilledRect(fillRect, SDL_Color{110, 210, 255, 220});
    }
}

Entity FluidManager::getFluidPipeAtTile(int tileX, int tileY) const {
    auto it = m_FluidPipeEntitiesByTile.find(makeTileKey(tileX, tileY));
    if (it == m_FluidPipeEntitiesByTile.end()) {
        return 0;
    }

    return it->second;
}

Entity FluidManager::getFluidTankAtTile(int tileX, int tileY) const {
    auto it = m_FluidTankEntitiesByTile.find(makeTileKey(tileX, tileY));
    if (it == m_FluidTankEntitiesByTile.end()) {
        return 0;
    }

    return it->second;
}

Entity FluidManager::getFluidPumpAtTile(int tileX, int tileY) const {
    auto it = m_FluidPumpEntitiesByTile.find(makeTileKey(tileX, tileY));
    if (it == m_FluidPumpEntitiesByTile.end()) {
        return 0;
    }

    return it->second;
}
