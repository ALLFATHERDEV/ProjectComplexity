#include "CraftingSystem.hpp"
#include <algorithm>

void CraftingSystem::update(float deltaTime, ComponentStorage<CraftingMachineComponent> &machines, ComponentStorage<MachineInventoryComponent> &inventories, const RecipeDatabase &recipeDatabase, const ItemDatabase &itemDatabase) {
    auto& machineArray = machines.getRaw();
    auto& entities = inventories.getEntities();

    for (size_t i = 0; i < machineArray.size(); i++) {
        Entity entity = entities[i];
        auto* machineInventory = inventories.get(entity);
        if (!machineInventory) continue;

        CraftingMachineComponent& machine = machineArray[i];
        const RecipeDefinition* recipe = recipeDatabase.getRecipe(machine.currentRecipeName);
        if (!recipe) continue;
        if (!syncRecipeInventoryLayout(*machineInventory, *recipe)) continue;

        processMachine(deltaTime, machine, *machineInventory, *recipe, itemDatabase);
    }
}

bool CraftingSystem::syncRecipeInventoryLayout(MachineInventoryComponent& inventory, const RecipeDefinition& recipe) {
    const int inputSlots = std::max(1, static_cast<int>(recipe.inputs.size()));
    const int outputSlots = recipe.outputItemName.empty() ? 0 : 1;

    if (!inventory.inputInventory.resizePreserve(inputSlots, 1)) {
        return false;
    }

    if (!inventory.outputInventory.resizePreserve(outputSlots, outputSlots > 0 ? 1 : 0)) {
        return false;
    }

    return true;
}

void CraftingSystem::processMachine(float deltaTime, CraftingMachineComponent &machine, MachineInventoryComponent &inventory, const RecipeDefinition &recipe, const ItemDatabase &itemDatabase) {
    if (!hasIngredients(inventory, recipe, itemDatabase)) {
        machine.progress = 0.0f;
        machine.isCrafting = false;
        return;
    }

    const ItemDefinition* outputItem = itemDatabase.getItem(recipe.outputItemName);
    if (!outputItem)
        return;
    if (!canOutputFit(inventory, outputItem, recipe.outputAmount)) {
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
        inventory.outputInventory.addItem(outputItem, recipe.outputAmount);

        machine.progress = 0.0f;
        machine.isCrafting = false;
    }
}

bool CraftingSystem::hasIngredients(MachineInventoryComponent &inventory, const RecipeDefinition &recipe, const ItemDatabase &itemDatabase) {
    for (const auto& ingredient : recipe.inputs) {
        const ItemDefinition* item = itemDatabase.getItem(ingredient.itemName);
        if (!item)
            return false;

        if (inventory.inputInventory.countItem(item) < ingredient.amount)
            return false;
    }
    return true;
}

void CraftingSystem::consumeIngredient(MachineInventoryComponent &inventory, const RecipeDefinition &recipe, const ItemDatabase &itemDatabase) {
    for (const auto& ingredient : recipe.inputs) {
        const ItemDefinition* item = itemDatabase.getItem(ingredient.itemName);
        if (!item)
            continue;

        inventory.inputInventory.removeItem(item, ingredient.amount);
    }
}

bool CraftingSystem::canOutputFit(MachineInventoryComponent &inventory, const ItemDefinition *outputItem, int amount) {
    InventoryGrid& output = inventory.outputInventory;
    if (output.getWidth() <= 0 || output.getHeight() <= 0) {
        return false;
    }

    int remaining = amount;

    for (const auto& slot : output.getSlots()) {
        if (slot.isEmpty())
            remaining -= outputItem->maxStackSize;
        else if (slot.stack.item == outputItem)
            remaining -= outputItem->maxStackSize - slot.stack.amount;

        if (remaining <= 0)
            return true;
    }
    return false;
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
