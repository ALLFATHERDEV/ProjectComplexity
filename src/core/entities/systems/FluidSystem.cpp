#include "FluidSystem.hpp"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "../../Logger.hpp"

void FluidSystem::update(float deltaTime,
                         ComponentStorage<PositionComponent> &positions,
                         ComponentStorage<FluidPipeComponent> &pipes,
                         ComponentStorage<FluidTankComponent> &tanks,
                         ComponentStorage<FluidPumpComponent> &pumps,
                         ComponentStorage<FluidPortComponent> &ports) {
    if (m_NetworksDirty) {
        rebuildNetworks(positions, pipes, tanks, ports);
    }

    refreshAllNetworkStorage(pipes, tanks);
    equalizeAllNetworks(pipes, tanks);
    refreshAllNetworkStorage(pipes, tanks);
    processPumps(deltaTime, pumps, pipes, tanks);
    refreshAllNetworkStorage(pipes, tanks);
    equalizeAllNetworks(pipes, tanks);
    refreshAllNetworkStorage(pipes, tanks);
}

void FluidSystem::markNetworksDirty() {
    m_NetworksDirty = true;
}

void FluidSystem::rebuildNetworks(ComponentStorage<PositionComponent> &positions,
                                  ComponentStorage<FluidPipeComponent> &pipes,
                                  ComponentStorage<FluidTankComponent> &tanks,
                                  ComponentStorage<FluidPortComponent> &ports) {
    m_Networks.clear();
    m_EntityToNetworkId.clear();
    m_NextNetworkId = 1;

    std::unordered_map<long long, Entity> pipeEntitiesByTile;
    std::unordered_map<long long, Entity> tankEntitiesByTile;
    std::unordered_map<long long, std::vector<Entity>> portEntitiesByTile;

    const std::vector<Entity> &pipeEntities = pipes.getEntities();
    for (Entity entity: pipeEntities) {
        const PositionComponent *position = positions.get(entity);
        if (!position) {
            continue;
        }

        const int tileX = worldToTile(position->position.x);
        const int tileY = worldToTile(position->position.y);
        pipeEntitiesByTile[makeTileKey(tileX, tileY)] = entity;
    }

    const std::vector<Entity> &tankEntities = tanks.getEntities();
    for (Entity entity: tankEntities) {
        const PositionComponent *position = positions.get(entity);
        if (!position) {
            continue;
        }

        const int tileX = worldToTile(position->position.x);
        const int tileY = worldToTile(position->position.y);
        tankEntitiesByTile[makeTileKey(tileX, tileY)] = entity;
    }

    const std::vector<Entity> &portEntities = ports.getEntities();
    for (Entity entity: portEntities) {
        const PositionComponent *position = positions.get(entity);
        if (!position) {
            continue;
        }

        const int tileX = worldToTile(position->position.x);
        const int tileY = worldToTile(position->position.y);
        portEntitiesByTile[makeTileKey(tileX, tileY)].push_back(entity);
    }

    std::unordered_set<Entity> visitedPipes;
    for (Entity startPipe: pipeEntities) {
        if (visitedPipes.contains(startPipe)) {
            continue;
        }

        FluidNetwork network;
        network.id = m_NextNetworkId++;

        std::unordered_set<Entity> networkTanks;
        std::unordered_set<Entity> networkPorts;
        std::queue<Entity> openPipes;
        openPipes.push(startPipe);
        visitedPipes.insert(startPipe);

        while (!openPipes.empty()) {
            const Entity currentPipe = openPipes.front();
            openPipes.pop();

            network.pipes.push_back(currentPipe);
            m_EntityToNetworkId[currentPipe] = network.id;

            const PositionComponent *pipePosition = positions.get(currentPipe);
            const FluidPipeComponent *pipeComponent = pipes.get(currentPipe);
            if (!pipePosition || !pipeComponent) {
                continue;
            }

            const int tileX = worldToTile(pipePosition->position.x);
            const int tileY = worldToTile(pipePosition->position.y);

            const std::pair<Direction, std::pair<int, int> > neighbors[] = {
                {Direction::UP, {tileX, tileY - 1}},
                {Direction::DOWN, {tileX, tileY + 1}},
                {Direction::LEFT, {tileX - 1, tileY}},
                {Direction::RIGHT, {tileX + 1, tileY}}
            };

            for (const auto &neighbor: neighbors) {
                const Direction direction = neighbor.first;
                const int neighborTileX = neighbor.second.first;
                const int neighborTileY = neighbor.second.second;
                const long long neighborKey = makeTileKey(neighborTileX, neighborTileY);

                auto pipeIt = pipeEntitiesByTile.find(neighborKey);
                if (pipeIt != pipeEntitiesByTile.end()) {
                    const Entity neighborPipe = pipeIt->second;
                    const FluidPipeComponent *neighborPipeComponent = pipes.get(neighborPipe);
                    if (neighborPipeComponent &&
                        arePipeNeighborsConnected(*pipeComponent, *neighborPipeComponent, direction) &&
                        !visitedPipes.contains(neighborPipe)) {
                        visitedPipes.insert(neighborPipe);
                        openPipes.push(neighborPipe);
                    }
                }

                auto tankIt = tankEntitiesByTile.find(neighborKey);
                if (tankIt != tankEntitiesByTile.end() && pipeHasConnection(*pipeComponent, direction)) {
                    const Entity tankEntity = tankIt->second;
                    if (!networkTanks.contains(tankEntity)) {
                        networkTanks.insert(tankEntity);
                        network.tanks.push_back(tankEntity);
                        m_EntityToNetworkId[tankEntity] = network.id;
                    }
                }

                auto portIt = portEntitiesByTile.find(neighborKey);
                if (portIt != portEntitiesByTile.end() && pipeHasConnection(*pipeComponent, direction)) {
                    for (const Entity portEntity : portIt->second) {
                        const FluidPortComponent *portComponent = ports.get(portEntity);
                        if (!portComponent) {
                            continue;
                        }

                        if (portComponent->side != getOppositeDirection(direction)) {
                            continue;
                        }

                        if (!networkPorts.contains(portEntity)) {
                            networkPorts.insert(portEntity);
                            network.ports.push_back(portEntity);
                            m_EntityToNetworkId[portEntity] = network.id;
                        }
                    }
                }
            }
        }

        collectNetworkStorage(network, pipes, tanks);
        LOG_INFO("FluidSystem: network {} rebuilt pipes={} tanks={} ports={} capacity={}",
                 network.id,
                 network.pipes.size(),
                 network.tanks.size(),
                 network.ports.size(),
                 network.totalCapacity);
        m_Networks.push_back(network);
    }

    LOG_INFO("FluidSystem: rebuilt {} networks", m_Networks.size());
    m_NetworksDirty = false;
}

const std::vector<FluidNetwork> &FluidSystem::getNetworks() const {
    return m_Networks;
}

int FluidSystem::getNetworkIdForEntity(Entity entity) const {
    auto it = m_EntityToNetworkId.find(entity);
    if (it == m_EntityToNetworkId.end()) {
        return -1;
    }

    return it->second;
}

const FluidDefinition* FluidSystem::getNetworkFluidForEntity(Entity entity) const {
    const int networkId = getNetworkIdForEntity(entity);
    if (networkId < 0) {
        return nullptr;
    }

    const FluidNetwork* network = getNetworkById(networkId);
    return network ? network->fluid.fluid : nullptr;
}

long long FluidSystem::makeTileKey(int tileX, int tileY) {
    const unsigned long long x = static_cast<unsigned int>(tileX);
    const unsigned long long y = static_cast<unsigned int>(tileY);
    return static_cast<long long>((x << 32) | y);
}

int FluidSystem::worldToTile(float worldCoordinate) {
    return static_cast<int>(std::floor(worldCoordinate / 32.0f));
}

Direction FluidSystem::getOppositeDirection(Direction direction) {
    switch (direction) {
        case Direction::UP:
            return Direction::DOWN;
        case Direction::DOWN:
            return Direction::UP;
        case Direction::LEFT:
            return Direction::RIGHT;
        case Direction::RIGHT:
        default:
            return Direction::LEFT;
    }
}

bool FluidSystem::pipeHasConnection(const FluidPipeComponent &pipe, Direction direction) {
    switch (direction) {
        case Direction::UP:
            return pipe.connectUp;
        case Direction::DOWN:
            return pipe.connectDown;
        case Direction::LEFT:
            return pipe.connectLeft;
        case Direction::RIGHT:
        default:
            return pipe.connectRight;
    }
}

bool FluidSystem::arePipeNeighborsConnected(const FluidPipeComponent &pipeA,
                                            const FluidPipeComponent &pipeB,
                                            Direction dirFromAToB) const {
    return pipeHasConnection(pipeA, dirFromAToB) &&
           pipeHasConnection(pipeB, getOppositeDirection(dirFromAToB));
}

void FluidSystem::collectNetworkStorage(FluidNetwork &network,
                                        ComponentStorage<FluidPipeComponent> &pipes,
                                        ComponentStorage<FluidTankComponent> &tanks) {
    network.totalCapacity = 0.0f;
    network.fluid.fluid = nullptr;
    network.fluid.amount = 0.0f;

    for (Entity pipeEntity: network.pipes) {
        FluidPipeComponent *pipe = pipes.get(pipeEntity);
        if (!pipe) {
            continue;
        }

        network.totalCapacity += pipe->capacity;

        if (pipe->storage.amount <= 0.0f || !pipe->storage.fluid) {
            continue;
        }

        if (!network.fluid.fluid) {
            network.fluid.fluid = pipe->storage.fluid;
        }

        if (network.fluid.fluid != pipe->storage.fluid) {
            continue;
        }

        network.fluid.amount += pipe->storage.amount;
    }

    for (Entity tankEntity: network.tanks) {
        FluidTankComponent *tank = tanks.get(tankEntity);
        if (!tank) {
            continue;
        }

        network.totalCapacity += tank->capacity;

        if (tank->storage.amount <= 0.0f || !tank->storage.fluid) {
            continue;
        }

        if (!network.fluid.fluid) {
            network.fluid.fluid = tank->storage.fluid;
        }

        if (network.fluid.fluid != tank->storage.fluid) {
            continue;
        }

        network.fluid.amount += tank->storage.amount;
    }
}

void FluidSystem::refreshAllNetworkStorage(ComponentStorage<FluidPipeComponent> &pipes,
                                           ComponentStorage<FluidTankComponent> &tanks) {
    for (FluidNetwork &network: m_Networks) {
        collectNetworkStorage(network, pipes, tanks);
    }
}

void FluidSystem::processPumps(float deltaTime,
                               ComponentStorage<FluidPumpComponent> &pumps,
                               ComponentStorage<FluidPipeComponent> &pipes,
                               ComponentStorage<FluidTankComponent> &tanks) {
    const std::vector<Entity> &pumpEntities = pumps.getEntities();
    for (Entity pumpEntity: pumpEntities) {
        FluidPumpComponent *pump = pumps.get(pumpEntity);
        if (!pump || !pump->outputFluid || pump->outputPerSecond <= 0.0f) {
            continue;
        }

        const int networkId = getNetworkIdForEntity(pumpEntity);
        if (networkId < 0) {
            continue;
        }

        FluidNetwork *targetNetwork = getMutableNetworkById(networkId);

        if (!targetNetwork) {
            continue;
        }

        const float amountToProduce = pump->outputPerSecond * deltaTime;
        const float currentAmount = targetNetwork->fluid.amount;
        const float nextAmount = std::min(targetNetwork->totalCapacity, currentAmount + amountToProduce);
        targetNetwork->fluid.fluid = pump->outputFluid;
        targetNetwork->fluid.amount = nextAmount;
        equalizeNetworkStorage(*targetNetwork, pipes, tanks);
    }
}

float FluidSystem::extractFromNetwork(Entity entity,
                                      const FluidDefinition* fluid,
                                      float amount,
                                      ComponentStorage<FluidPipeComponent>& pipes,
                                      ComponentStorage<FluidTankComponent>& tanks) {
    if (!fluid || amount <= 0.0f) {
        return 0.0f;
    }

    FluidNetwork* network = getMutableNetworkById(getNetworkIdForEntity(entity));
    if (!network || network->fluid.fluid != fluid || network->fluid.amount <= 0.0f) {
        return 0.0f;
    }

    const float extracted = std::min(amount, network->fluid.amount);
    network->fluid.amount -= extracted;
    if (network->fluid.amount <= 0.0f) {
        network->fluid.amount = 0.0f;
        network->fluid.fluid = nullptr;
    }
    equalizeNetworkStorage(*network, pipes, tanks);
    return extracted;
}

float FluidSystem::insertIntoNetwork(Entity entity,
                                     const FluidDefinition* fluid,
                                     float amount,
                                     ComponentStorage<FluidPipeComponent>& pipes,
                                     ComponentStorage<FluidTankComponent>& tanks) {
    if (!fluid || amount <= 0.0f) {
        return 0.0f;
    }

    FluidNetwork* network = getMutableNetworkById(getNetworkIdForEntity(entity));
    if (!network) {
        return 0.0f;
    }

    if (network->fluid.fluid && network->fluid.fluid != fluid) {
        return 0.0f;
    }

    const float freeCapacity = std::max(0.0f, network->totalCapacity - network->fluid.amount);
    if (freeCapacity <= 0.0f) {
        return 0.0f;
    }

    const float inserted = std::min(amount, freeCapacity);
    network->fluid.fluid = fluid;
    network->fluid.amount += inserted;
    equalizeNetworkStorage(*network, pipes, tanks);
    return inserted;
}

float FluidSystem::fillNetworkTanks(FluidNetwork &network,
                                    const FluidDefinition *fluid,
                                    float amount,
                                    ComponentStorage<FluidTankComponent> &tanks) {
    if (!fluid || amount <= 0.0f) {
        return 0.0f;
    }

    if (network.fluid.fluid && network.fluid.fluid != fluid) {
        return 0.0f;
    }

    float remaining = amount;
    for (Entity tankEntity: network.tanks) {
        FluidTankComponent *tank = tanks.get(tankEntity);
        if (!tank) {
            continue;
        }

        if (tank->storage.fluid && tank->storage.fluid != fluid) {
            continue;
        }

        const float freeCapacity = tank->capacity - tank->storage.amount;
        if (freeCapacity <= 0.0f) {
            continue;
        }

        const float toFill = std::min(freeCapacity, remaining);
        tank->storage.fluid = fluid;
        tank->storage.amount += toFill;
        remaining -= toFill;

        if (remaining <= 0.0f) {
            break;
        }
    }

    return amount - remaining;
}

void FluidSystem::equalizeNetworkStorage(FluidNetwork &network,
                                         ComponentStorage<FluidPipeComponent> &pipes,
                                         ComponentStorage<FluidTankComponent> &tanks) {
    if (network.totalCapacity <= 0.0f) {
        return;
    }

    const FluidDefinition *fluid = network.fluid.fluid;
    const float clampedAmount = std::clamp(network.fluid.amount, 0.0f, network.totalCapacity);
    const float fillRatio = clampedAmount / network.totalCapacity;

    for (Entity pipeEntity: network.pipes) {
        FluidPipeComponent *pipe = pipes.get(pipeEntity);
        if (!pipe) {
            continue;
        }

        pipe->storage.fluid = fluid;
        pipe->storage.amount = fluid ? pipe->capacity * fillRatio : 0.0f;
        if (pipe->storage.amount <= 0.0f) {
            pipe->storage.fluid = nullptr;
        }
    }

    for (Entity tankEntity: network.tanks) {
        FluidTankComponent *tank = tanks.get(tankEntity);
        if (!tank) {
            continue;
        }

        tank->storage.fluid = fluid;
        tank->storage.amount = fluid ? tank->capacity * fillRatio : 0.0f;
        if (tank->storage.amount <= 0.0f) {
            tank->storage.fluid = nullptr;
        }
    }
}

void FluidSystem::equalizeAllNetworks(ComponentStorage<FluidPipeComponent> &pipes,
                                      ComponentStorage<FluidTankComponent> &tanks) {
    for (FluidNetwork &network: m_Networks) {
        equalizeNetworkStorage(network, pipes, tanks);
    }
}

FluidNetwork* FluidSystem::getMutableNetworkById(int id) {
    if (id < 0) {
        return nullptr;
    }

    for (FluidNetwork& network : m_Networks) {
        if (network.id == id) {
            return &network;
        }
    }

    return nullptr;
}

const FluidNetwork* FluidSystem::getNetworkById(int id) const {
    if (id < 0) {
        return nullptr;
    }

    for (const FluidNetwork& network : m_Networks) {
        if (network.id == id) {
            return &network;
        }
    }

    return nullptr;
}
