#pragma once

#include <unordered_map>
#include <vector>

#include "../../fluid/FluidNetwork.hpp"
#include "../ComponentStorage.hpp"
#include "../component/FluidPipeComponent.hpp"
#include "../component/FluidPumpComponent.hpp"
#include "../component/FluidPortComponent.hpp"
#include "../component/FluidTankComponent.hpp"
#include "../component/PositionComponent.hpp"

class FluidSystem {
public:
    void update(float deltaTime,
                ComponentStorage<PositionComponent>& positions,
                ComponentStorage<FluidPipeComponent>& pipes,
                ComponentStorage<FluidTankComponent>& tanks,
                ComponentStorage<FluidPumpComponent>& pumps,
                ComponentStorage<FluidPortComponent>& ports);

    void markNetworksDirty();
    void rebuildNetworks(ComponentStorage<PositionComponent>& positions,
                         ComponentStorage<FluidPipeComponent>& pipes,
                         ComponentStorage<FluidTankComponent>& tanks,
                         ComponentStorage<FluidPortComponent>& ports);

    const std::vector<FluidNetwork>& getNetworks() const;
    int getNetworkIdForEntity(Entity entity) const;

private:
    static long long makeTileKey(int tileX, int tileY);
    static int worldToTile(float worldCoordinate);
    static Direction getOppositeDirection(Direction direction);
    static bool pipeHasConnection(const FluidPipeComponent& pipe, Direction direction);

    bool arePipeNeighborsConnected(const FluidPipeComponent& pipeA,
                                   const FluidPipeComponent& pipeB,
                                   Direction dirFromAToB) const;

    void collectNetworkStorage(FluidNetwork& network,
                               ComponentStorage<FluidPipeComponent>& pipes,
                               ComponentStorage<FluidTankComponent>& tanks);
    void refreshAllNetworkStorage(ComponentStorage<FluidPipeComponent>& pipes, ComponentStorage<FluidTankComponent>& tanks);
    void processPumps(float deltaTime,
                      ComponentStorage<FluidPumpComponent>& pumps,
                      ComponentStorage<FluidPipeComponent>& pipes,
                      ComponentStorage<FluidTankComponent>& tanks);
    float fillNetworkTanks(FluidNetwork& network,
                           const FluidDefinition* fluid,
                           float amount,
                           ComponentStorage<FluidTankComponent>& tanks);
    void equalizeNetworkStorage(FluidNetwork& network,
                                ComponentStorage<FluidPipeComponent>& pipes,
                                ComponentStorage<FluidTankComponent>& tanks);
    void equalizeAllNetworks(ComponentStorage<FluidPipeComponent>& pipes,
                             ComponentStorage<FluidTankComponent>& tanks);

    std::vector<FluidNetwork> m_Networks;
    bool m_NetworksDirty = true;
    int m_NextNetworkId = 1;
    std::unordered_map<Entity, int> m_EntityToNetworkId;
};
