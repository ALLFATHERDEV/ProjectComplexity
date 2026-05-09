#pragma once
#include "../../entities/ComponentStorage.hpp"
#include "../../entities/component/CraftingMachineComponent.hpp"
#include "../../entities/component/MachineFluidComponent.hpp"
#include "../../entities/component/MachineInventoryComponent.hpp"

struct CraftingContext {
    ComponentStorage<CraftingMachineComponent>& craftingMachines;
    ComponentStorage<MachineInventoryComponent>& machineInventories;
    ComponentStorage<MachineFluidComponent>& machineFluids;
};
