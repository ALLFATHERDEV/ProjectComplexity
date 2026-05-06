#include "CraftingSystem.hpp"

#include <algorithm>

#include "../../Logger.hpp"

void CraftingSystem::update(float deltaTime,
                            ComponentStorage<CraftingMachineComponent>& machines,
                            ComponentStorage<MachineInventoryComponent>& inventories,
                            ComponentStorage<MachineFluidComponent>& machineFluids,
                            const RecipeDatabase& recipeDatabase,
                            const ItemDatabase& itemDatabase,
                            const FluidDatabase& fluidDatabase) {
    auto& machineArray = machines.getRaw();
    auto& entities = machines.getEntities();

    for (size_t i = 0; i < machineArray.size(); i++) {
        Entity entity = entities[i];
        auto* machineInventory = inventories.get(entity);
        if (!machineInventory) {
            LOG_WARN("CraftingSystem: machine entity {} has no MachineInventoryComponent", entity);
            continue;
        }

        CraftingMachineComponent& machine = machineArray[i];
        const RecipeDefinition* recipe = recipeDatabase.getRecipe(machine.currentRecipeName);
        if (!recipe) {
            LOG_WARN("CraftingSystem: machine {} references missing recipe {}", machine.machineUniqueName, machine.currentRecipeName);
            continue;
        }

        if (!syncRecipeInventoryLayout(*machineInventory, *recipe)) {
            LOG_ERROR("CraftingSystem: failed to resize inventories for machine {} recipe {}", machine.machineUniqueName, recipe->uniqueName);
            continue;
        }

        processMachine(deltaTime,
                       machine,
                       *machineInventory,
                       machineFluids.get(entity),
                       *recipe,
                       itemDatabase,
                       fluidDatabase);
    }
}

bool CraftingSystem::syncRecipeInventoryLayout(MachineInventoryComponent& inventory, const RecipeDefinition& recipe) {
    const int inputSlots = std::max(1, static_cast<int>(recipe.itemInputs.size()));
    const int outputSlots = static_cast<int>(recipe.itemOutputs.size());

    if (!inventory.inputInventory.resizePreserve(inputSlots, 1)) {
        return false;
    }

    if (!inventory.outputInventory.resizePreserve(outputSlots, outputSlots > 0 ? 1 : 0)) {
        return false;
    }

    return true;
}

void CraftingSystem::processMachine(float deltaTime,
                                    CraftingMachineComponent& machine,
                                    MachineInventoryComponent& inventory,
                                    MachineFluidComponent* machineFluid,
                                    const RecipeDefinition& recipe,
                                    const ItemDatabase& itemDatabase,
                                    const FluidDatabase& fluidDatabase) {
    if (!hasIngredients(inventory, recipe, itemDatabase) ||
        !hasFluidIngredients(machineFluid, recipe, fluidDatabase)) {
        machine.progress = 0.0f;
        machine.isCrafting = false;
        return;
    }

    if (!canOutputsFit(inventory, machineFluid, recipe, itemDatabase, fluidDatabase)) {
        machine.isCrafting = false;
        return;
    }

    if (machine.requiresFuel && machine.fuelRemaining <= 0.0f) {
        if (!tryConsumeFuel(machine, inventory)) {
            machine.isCrafting = false;
            return;
        }
    }

    machine.isCrafting = true;
    if (machine.requiresFuel) {
        machine.fuelRemaining -= deltaTime;
        if (machine.fuelRemaining < 0.0f) {
            machine.fuelRemaining = 0.0f;
        }
    }
    machine.progress += deltaTime;

    if (machine.progress >= recipe.craftTime) {
        consumeIngredient(inventory, recipe, itemDatabase);
        consumeFluidIngredients(machineFluid, recipe, fluidDatabase);
        addOutputs(inventory, machineFluid, recipe, itemDatabase, fluidDatabase);

        LOG_INFO("Craft complete: machine={} recipe={}", machine.machineUniqueName, recipe.uniqueName);

        machine.progress = 0.0f;
        machine.isCrafting = false;
    }
}

bool CraftingSystem::hasIngredients(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase) {
    for (size_t i = 0; i < recipe.itemInputs.size(); i++) {
        const auto& ingredient = recipe.itemInputs[i];
        const ItemDefinition* item = itemDatabase.getItem(ingredient.itemName);
        if (!item) {
            return false;
        }

        InventorySlot* inputSlot = inventory.inputInventory.getSlot(static_cast<int>(i), 0);
        if (!inputSlot || inputSlot->isEmpty()) {
            return false;
        }

        if (inputSlot->stack.item != item) {
            return false;
        }

        if (inputSlot->stack.amount < ingredient.amount) {
            return false;
        }
    }
    return true;
}

bool CraftingSystem::hasFluidIngredients(MachineFluidComponent* machineFluid,
                                         const RecipeDefinition& recipe,
                                         const FluidDatabase& fluidDatabase) {
    if (recipe.fluidInputs.empty()) {
        return true;
    }

    if (!machineFluid || machineFluid->inputs.size() < recipe.fluidInputs.size()) {
        if (machineFluid) {
            LOG_WARN("CraftingSystem: machine fluid inputs missing slots required={} actual={}",
                     recipe.fluidInputs.size(),
                     machineFluid->inputs.size());
        }
        return false;
    }

    for (size_t i = 0; i < recipe.fluidInputs.size(); i++) {
        const auto& ingredient = recipe.fluidInputs[i];
        const FluidDefinition* fluid = fluidDatabase.getFluid(ingredient.fluidName);
        if (!fluid) {
            return false;
        }

        MachineFluidSlot* slot = findInputFluidSlot(machineFluid, ingredient, i);
        if (!slot || slot->storage.fluid != fluid || slot->storage.amount < ingredient.amount) {
            return false;
        }
    }

    return true;
}

void CraftingSystem::consumeIngredient(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase) {
    for (size_t i = 0; i < recipe.itemInputs.size(); i++) {
        const auto& ingredient = recipe.itemInputs[i];
        const ItemDefinition* item = itemDatabase.getItem(ingredient.itemName);
        if (!item) {
            continue;
        }

        InventorySlot* inputSlot = inventory.inputInventory.getSlot(static_cast<int>(i), 0);
        if (!inputSlot || inputSlot->isEmpty() || inputSlot->stack.item != item) {
            continue;
        }

        inputSlot->stack.amount -= ingredient.amount;
        if (inputSlot->stack.amount <= 0) {
            inputSlot->stack.clear();
        }
    }
}

void CraftingSystem::consumeFluidIngredients(MachineFluidComponent* machineFluid,
                                             const RecipeDefinition& recipe,
                                             const FluidDatabase& fluidDatabase) {
    if (!machineFluid) {
        return;
    }

    for (size_t i = 0; i < recipe.fluidInputs.size() && i < machineFluid->inputs.size(); i++) {
        const auto& ingredient = recipe.fluidInputs[i];
        const FluidDefinition* fluid = fluidDatabase.getFluid(ingredient.fluidName);
        if (!fluid) {
            continue;
        }

        MachineFluidSlot* slot = findInputFluidSlot(machineFluid, ingredient, i);
        if (!slot || slot->storage.fluid != fluid) {
            continue;
        }

        slot->storage.amount -= ingredient.amount;
        if (slot->storage.amount <= 0.0f) {
            slot->storage.amount = 0.0f;
            slot->storage.fluid = nullptr;
        }
    }
}

bool CraftingSystem::canOutputsFit(MachineInventoryComponent& inventory,
                                   MachineFluidComponent* machineFluid,
                                   const RecipeDefinition& recipe,
                                   const ItemDatabase& itemDatabase,
                                   const FluidDatabase& fluidDatabase) {
    InventoryGrid outputCopy = inventory.outputInventory;

    for (const auto& recipeOutput : recipe.itemOutputs) {
        const ItemDefinition* outputItem = itemDatabase.getItem(recipeOutput.itemName);
        if (!outputItem) {
            return false;
        }

        if (!outputCopy.addItem(outputItem, recipeOutput.amount)) {
            return false;
        }
    }

    if (!recipe.fluidOutputs.empty()) {
        if (!machineFluid || machineFluid->outputs.size() < recipe.fluidOutputs.size()) {
            if (machineFluid) {
                LOG_WARN("CraftingSystem: machine fluid outputs missing slots required={} actual={}",
                         recipe.fluidOutputs.size(),
                         machineFluid->outputs.size());
            }
            return false;
        }

        for (size_t i = 0; i < recipe.fluidOutputs.size(); i++) {
            const auto& recipeOutput = recipe.fluidOutputs[i];
            const FluidDefinition* outputFluid = fluidDatabase.getFluid(recipeOutput.fluidName);
            if (!outputFluid) {
                return false;
            }

            MachineFluidSlot* slot = findOutputFluidSlot(machineFluid, recipeOutput, i);
            if (!slot) {
                return false;
            }

            if (slot->storage.fluid && slot->storage.fluid != outputFluid) {
                return false;
            }

            if (slot->storage.amount + recipeOutput.amount > slot->capacity) {
                return false;
            }
        }
    }

    return true;
}

void CraftingSystem::addOutputs(MachineInventoryComponent& inventory,
                                MachineFluidComponent* machineFluid,
                                const RecipeDefinition& recipe,
                                const ItemDatabase& itemDatabase,
                                const FluidDatabase& fluidDatabase) {
    for (const auto& recipeOutput : recipe.itemOutputs) {
        const ItemDefinition* outputItem = itemDatabase.getItem(recipeOutput.itemName);
        if (!outputItem) {
            continue;
        }

        inventory.outputInventory.addItem(outputItem, recipeOutput.amount);
    }

    if (!machineFluid) {
        return;
    }

    for (size_t i = 0; i < recipe.fluidOutputs.size() && i < machineFluid->outputs.size(); i++) {
        const auto& recipeOutput = recipe.fluidOutputs[i];
        const FluidDefinition* outputFluid = fluidDatabase.getFluid(recipeOutput.fluidName);
        if (!outputFluid) {
            continue;
        }

        MachineFluidSlot* slot = findOutputFluidSlot(machineFluid, recipeOutput, i);
        if (!slot) {
            continue;
        }

        slot->storage.fluid = outputFluid;
        slot->storage.amount += recipeOutput.amount;
    }
}

MachineFluidSlot* CraftingSystem::findInputFluidSlot(MachineFluidComponent* machineFluid,
                                                     const RecipeFluidStack& fluidStack,
                                                     size_t fallbackIndex) const {
    if (!machineFluid) {
        return nullptr;
    }

    return findFluidSlot(machineFluid->inputs, fluidStack, fallbackIndex);
}

MachineFluidSlot* CraftingSystem::findOutputFluidSlot(MachineFluidComponent* machineFluid,
                                                      const RecipeFluidStack& fluidStack,
                                                      size_t fallbackIndex) const {
    if (!machineFluid) {
        return nullptr;
    }

    return findFluidSlot(machineFluid->outputs, fluidStack, fallbackIndex);
}

MachineFluidSlot* CraftingSystem::findFluidSlot(std::vector<MachineFluidSlot>& slots,
                                                const RecipeFluidStack& fluidStack,
                                                size_t fallbackIndex) const {
    if (!fluidStack.slotName.empty()) {
        for (MachineFluidSlot& slot : slots) {
            if (slot.name == fluidStack.slotName) {
                return &slot;
            }
        }

        LOG_WARN("CraftingSystem: no fluid slot found for slotName={}", fluidStack.slotName);
        return nullptr;
    }

    if (fallbackIndex >= slots.size()) {
        return nullptr;
    }

    return &slots[fallbackIndex];
}

bool CraftingSystem::tryConsumeFuel(CraftingMachineComponent& machine, MachineInventoryComponent& inventory) {
    for (auto& slot : inventory.fuelInventory.getSlots()) {
        if (slot.isEmpty() || !slot.stack.item || slot.stack.item->fuelValue <= 0.0f) {
            continue;
        }

        const ItemDefinition* fuelItem = slot.stack.item;
        if (!inventory.fuelInventory.removeItem(fuelItem, 1)) {
            continue;
        }

        machine.currentFuelCapacity = fuelItem->fuelValue;
        machine.fuelRemaining = fuelItem->fuelValue;
        return true;
    }

    return false;
}
