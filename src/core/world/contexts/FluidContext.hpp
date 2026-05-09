#pragma once
#include "../../entities/ComponentStorage.hpp"

struct FluidPortComponent;
struct FluidPumpComponent;
struct FluidTankComponent;
struct FluidPipeComponent;
struct PositionComponent;

struct FluidContext {
    ComponentStorage<PositionComponent>& positions;
    ComponentStorage<FluidPipeComponent>& fluidPipes;
    ComponentStorage<FluidTankComponent>& fluidTanks;
    ComponentStorage<FluidPumpComponent>& fluidPumps;
    ComponentStorage<FluidPortComponent>& fluidPorts;
};
