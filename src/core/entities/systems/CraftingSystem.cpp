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
    const int outputSlots = static_cast<int>(recipe.outputs.size());

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

    if (!canOutputsFit(inventory, recipe, itemDatabase)) {
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
        addOutputs(inventory, recipe, itemDatabase);

        machine.progress = 0.0f;
        machine.isCrafting = false;
    }
}

bool CraftingSystem::hasIngredients(MachineInventoryComponent &inventory, const RecipeDefinition &recipe, const ItemDatabase &itemDatabase) {
    for (size_t i = 0; i < recipe.inputs.size(); i++) {
        const auto& ingredient = recipe.inputs[i];
        const ItemDefinition* item = itemDatabase.getItem(ingredient.itemName);
        if (!item)
            return false;

        InventorySlot* inputSlot = inventory.inputInventory.getSlot(static_cast<int>(i), 0);
        if (!inputSlot || inputSlot->isEmpty())
            return false;

        if (inputSlot->stack.item != item)
            return false;

        if (inputSlot->stack.amount < ingredient.amount)
            return false;
    }
    return true;
}

void CraftingSystem::consumeIngredient(MachineInventoryComponent &inventory, const RecipeDefinition &recipe, const ItemDatabase &itemDatabase) {
    for (size_t i = 0; i < recipe.inputs.size(); i++) {
        const auto& ingredient = recipe.inputs[i];
        const ItemDefinition* item = itemDatabase.getItem(ingredient.itemName);
        if (!item)
            continue;

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

bool CraftingSystem::canOutputsFit(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase) {
    InventoryGrid outputCopy = inventory.outputInventory;

    for (const auto& recipeOutput : recipe.outputs) {
        const ItemDefinition* outputItem = itemDatabase.getItem(recipeOutput.itemName);
        if (!outputItem) {
            return false;
        }

        if (!outputCopy.addItem(outputItem, recipeOutput.amount)) {
            return false;
        }
    }

    return true;
}

void CraftingSystem::addOutputs(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase) {
    for (const auto& recipeOutput : recipe.outputs) {
        const ItemDefinition* outputItem = itemDatabase.getItem(recipeOutput.itemName);
        if (!outputItem) {
            continue;
        }

        inventory.outputInventory.addItem(outputItem, recipeOutput.amount);
    }
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
