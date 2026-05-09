#pragma once
#include "../../entities/ComponentStorage.hpp"
#include "../../entities/component/MachineFluidPortLinkComponent.hpp"

struct FluidTankComponent;
struct FluidPipeComponent;
struct MachineFluidComponent;
struct CraftingMachineComponent;

struct MachineFluidIOContext {
    ComponentStorage<MachineFluidPortLinkComponent>& machineFluidPortLinks;
    ComponentStorage<FluidPortComponent>& fluidPorts;
    ComponentStorage<CraftingMachineComponent>& craftingMachines;
    ComponentStorage<MachineFluidComponent>& machineFluids;
    ComponentStorage<FluidPipeComponent>& fluidPipes;
    ComponentStorage<FluidTankComponent>& fluidTanks;
};
