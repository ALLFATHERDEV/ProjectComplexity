#include "MachineFluidIOSystem.hpp"

#include <algorithm>

void MachineFluidIOSystem::update(float deltaTime,
                                  ComponentStorage<MachineFluidPortLinkComponent>& portLinks,
                                  ComponentStorage<FluidPortComponent>& ports,
                                  ComponentStorage<CraftingMachineComponent>& craftingMachines,
                                  ComponentStorage<MachineFluidComponent>& machineFluids,
                                  ComponentStorage<FluidPipeComponent>& pipes,
                                  ComponentStorage<FluidTankComponent>& tanks,
                                  const RecipeDatabase& recipeDatabase,
                                  const FluidDatabase& fluidDatabase,
                                  FluidSystem& fluidSystem) {
    auto& linkArray = portLinks.getRaw();
    auto& entities = portLinks.getEntities();

    for (size_t i = 0; i < linkArray.size(); i++) {
        const Entity portEntity = entities[i];
        const MachineFluidPortLinkComponent& link = linkArray[i];
        CraftingMachineComponent* machine = craftingMachines.get(link.machineEntity);
        FluidPortComponent* port = ports.get(portEntity);
        MachineFluidComponent* machineFluid = machineFluids.get(link.machineEntity);
        if (!port || !machineFluid) {
            continue;
        }

        MachineFluidSlot* slot = findSlot(*machineFluid, link);
        if (!slot || slot->capacity <= 0.0f) {
            continue;
        }

        const float transferLimit = std::max(0.0f, port->maxTransferPerSecond * deltaTime);
        if (transferLimit <= 0.0f) {
            continue;
        }

        if (link.type == FluidPortType::INPUT) {
            const FluidDefinition* allowedFluid = nullptr;
            if (!tryGetAllowedInputFluid(link, craftingMachines, recipeDatabase, fluidDatabase, allowedFluid)) {
                continue;
            }

            const FluidDefinition* networkFluid = fluidSystem.getNetworkFluidForEntity(portEntity);
            if (!networkFluid) {
                continue;
            }

            if (!allowedFluid || allowedFluid != networkFluid) {
                continue;
            }

            if (slot->storage.fluid && slot->storage.fluid != networkFluid) {
                continue;
            }

            const float freeCapacity = std::max(0.0f, slot->capacity - slot->storage.amount);
            if (freeCapacity <= 0.0f) {
                continue;
            }

            const float extracted = fluidSystem.extractFromNetwork(portEntity,
                                                                   networkFluid,
                                                                   std::min(freeCapacity, transferLimit),
                                                                   pipes,
                                                                   tanks);
            if (extracted <= 0.0f) {
                continue;
            }

            slot->storage.fluid = networkFluid;
            slot->storage.amount += extracted;
            continue;
        }

        if (link.type == FluidPortType::OUTPUT) {
            if (!slot->storage.fluid || slot->storage.amount <= 0.0f) {
                continue;
            }

            const float inserted = fluidSystem.insertIntoNetwork(portEntity,
                                                                 slot->storage.fluid,
                                                                 std::min(slot->storage.amount, transferLimit),
                                                                 pipes,
                                                                 tanks);
            if (inserted <= 0.0f) {
                continue;
            }

            slot->storage.amount -= inserted;
            if (slot->storage.amount <= 0.0f) {
                slot->storage.amount = 0.0f;
                slot->storage.fluid = nullptr;
            }
        }
    }
}

MachineFluidSlot* MachineFluidIOSystem::findSlot(MachineFluidComponent& machineFluid, const MachineFluidPortLinkComponent& link) const {
    auto& slots = link.type == FluidPortType::OUTPUT ? machineFluid.outputs : machineFluid.inputs;
    for (MachineFluidSlot& slot : slots) {
        if (slot.name == link.slotName) {
            return &slot;
        }
    }

    return nullptr;
}

bool MachineFluidIOSystem::tryGetAllowedInputFluid(const MachineFluidPortLinkComponent& link,
                                                   ComponentStorage<CraftingMachineComponent>& craftingMachines,
                                                   const RecipeDatabase& recipeDatabase,
                                                   const FluidDatabase& fluidDatabase,
                                                   const FluidDefinition*& allowedFluid) const {
    allowedFluid = nullptr;

    CraftingMachineComponent* machine = craftingMachines.get(link.machineEntity);
    if (!machine) {
        return false;
    }

    const RecipeDefinition* recipe = recipeDatabase.getRecipe(machine->currentRecipeName);
    if (!recipe) {
        return false;
    }

    if (recipe->fluidInputs.empty()) {
        return false;
    }

    for (const RecipeFluidStack& fluidInput : recipe->fluidInputs) {
        if (!fluidInput.slotName.empty() && fluidInput.slotName == link.slotName) {
            allowedFluid = fluidDatabase.getFluid(fluidInput.fluidName);
            return allowedFluid != nullptr;
        }
    }

    return false;
}
