#include "FluidManager.hpp"

#include <algorithm>

#include "ChunkManager.hpp"
#include "EntityFactory.hpp"

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

long long FluidManager::makeTileKey(int tileX, int tileY) {
    const unsigned long long x = static_cast<unsigned int>(tileX);
    const unsigned long long y = static_cast<unsigned int>(tileY);
    return static_cast<long long>((x << 32) | y);
}

Sprite FluidManager::getPipeSprite(Direction direction) const {
    if (!m_FluidAtlas) {
        return {nullptr, {0.0f, 0.0f, 0.0f, 0.0f}};
    }

    switch (direction) {
        case Direction::UP:
        case Direction::DOWN:
            return m_FluidAtlas->getSprite(0, 0);
        case Direction::LEFT:
        case Direction::RIGHT:
        default:
            return m_FluidAtlas->getSprite(1, 0);
    }
}

Sprite FluidManager::getTankSprite() const {
    if (!m_FluidAtlas) {
        return {nullptr, {0.0f, 0.0f, 0.0f, 0.0f}};
    }

    return m_FluidAtlas->getSprite(0, 1);
}

Sprite FluidManager::getPumpSprite(Direction direction) const {
    if (!m_FluidAtlas) {
        return {nullptr, {0.0f, 0.0f, 0.0f, 0.0f}};
    }

    switch (direction) {
        case Direction::UP:
            return m_FluidAtlas->getSprite(6, 2);
        case Direction::DOWN:
            return m_FluidAtlas->getSprite(6, 2);
        case Direction::LEFT:
            return m_FluidAtlas->getSprite(6, 2);
        case Direction::RIGHT:
        default:
            return m_FluidAtlas->getSprite(6, 2);
    }
}

void FluidManager::refreshFluidSpriteAt(int tileX, int tileY) {
    if (const Entity pipe = getFluidPipeAtTile(tileX, tileY); pipe != 0) {
        if (SpriteComponent* sprite = m_Sprites.get(pipe)) {
            Direction direction = Direction::RIGHT;
            if (const FluidPipeComponent* pipeComponent = m_FluidPipes.get(pipe)) {
                direction = pipeComponent->direction;
            }
            sprite->sprite = getPipeSprite(direction);
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

    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_FluidPipes, m_FluidTanks, m_FluidPumps, m_FluidPorts, m_Machines, m_MachineInventories, m_CraftingMachines, m_Miners, m_Interactions, m_AnimationLibrary);

    const Entity pipe = factory.createFluidPipe(
        {static_cast<float>(tileX) * kTileSize, static_cast<float>(tileY) * kTileSize},
        getPipeSprite(direction),
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

    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_FluidPipes, m_FluidTanks, m_FluidPumps, m_FluidPorts, m_Machines, m_MachineInventories, m_CraftingMachines, m_Miners, m_Interactions, m_AnimationLibrary);

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
    m_FluidTanks.remove(tank);
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

    EntityFactory factory(m_EntityManager, m_Positions, m_Velocities, m_Inputs, m_CharacterStates, m_AnimationControllers, m_Sprites, m_Collisions, m_ConveyorBelts, m_Inventories, m_FluidPipes, m_FluidTanks, m_FluidPumps, m_FluidPorts, m_Machines, m_MachineInventories, m_CraftingMachines, m_Miners, m_Interactions, m_AnimationLibrary);

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
    m_FluidPumps.remove(pump);
    m_FluidPorts.remove(pump);
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

    if (tankComponent->storage.fluid && tankComponent->storage.fluid != &m_DebugWater) {
        return false;
    }

    tankComponent->storage.fluid = &m_DebugWater;
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
