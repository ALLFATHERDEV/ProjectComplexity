#pragma once
#include "../ComponentStorage.hpp"
#include "../../inventory/ItemDatabase.hpp"
#include "../../recipe/RecipeDatabase.hpp"
#include "../component/CraftingMachineComponent.hpp"
#include "../component/MachineInventoryComponent.hpp"

class CraftingSystem {
public:
    void update(float deltaTime, ComponentStorage<CraftingMachineComponent>& machines, ComponentStorage<MachineInventoryComponent>& inventories, const RecipeDatabase& recipeDatabase, const ItemDatabase& itemDatabase);

private:
    bool syncRecipeInventoryLayout(MachineInventoryComponent& inventory, const RecipeDefinition& recipe);
    void processMachine(float deltaTime, CraftingMachineComponent& machine, MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase);
    bool hasIngredients(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase);
    void consumeIngredient(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase);
    bool canOutputsFit(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase);
    void addOutputs(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase);
    bool tryConsumeFuel(CraftingMachineComponent& machine, MachineInventoryComponent& inventory);
};
