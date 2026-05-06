#pragma once
#include "../ComponentStorage.hpp"
#include "../../inventory/ItemDatabase.hpp"
#include "../../fluid/FluidDatabase.hpp"
#include "../../recipe/RecipeDatabase.hpp"
#include "../component/CraftingMachineComponent.hpp"
#include "../component/MachineFluidComponent.hpp"
#include "../component/MachineInventoryComponent.hpp"

class CraftingSystem {
public:
    void update(float deltaTime,
                ComponentStorage<CraftingMachineComponent>& machines,
                ComponentStorage<MachineInventoryComponent>& inventories,
                ComponentStorage<MachineFluidComponent>& machineFluids,
                const RecipeDatabase& recipeDatabase,
                const ItemDatabase& itemDatabase,
                const FluidDatabase& fluidDatabase);

private:
    bool syncRecipeInventoryLayout(MachineInventoryComponent& inventory, const RecipeDefinition& recipe);
    void processMachine(float deltaTime,
                        CraftingMachineComponent& machine,
                        MachineInventoryComponent& inventory,
                        MachineFluidComponent* machineFluid,
                        const RecipeDefinition& recipe,
                        const ItemDatabase& itemDatabase,
                        const FluidDatabase& fluidDatabase);
    MachineFluidSlot* findInputFluidSlot(MachineFluidComponent* machineFluid, const RecipeFluidStack& fluidStack, size_t fallbackIndex) const;
    MachineFluidSlot* findOutputFluidSlot(MachineFluidComponent* machineFluid, const RecipeFluidStack& fluidStack, size_t fallbackIndex) const;
    MachineFluidSlot* findFluidSlot(std::vector<MachineFluidSlot>& slots, const RecipeFluidStack& fluidStack, size_t fallbackIndex) const;
    bool hasIngredients(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase);
    bool hasFluidIngredients(MachineFluidComponent* machineFluid, const RecipeDefinition& recipe, const FluidDatabase& fluidDatabase);
    void consumeIngredient(MachineInventoryComponent& inventory, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase);
    void consumeFluidIngredients(MachineFluidComponent* machineFluid, const RecipeDefinition& recipe, const FluidDatabase& fluidDatabase);
    bool canOutputsFit(MachineInventoryComponent& inventory, MachineFluidComponent* machineFluid, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase, const FluidDatabase& fluidDatabase);
    void addOutputs(MachineInventoryComponent& inventory, MachineFluidComponent* machineFluid, const RecipeDefinition& recipe, const ItemDatabase& itemDatabase, const FluidDatabase& fluidDatabase);
    bool tryConsumeFuel(CraftingMachineComponent& machine, MachineInventoryComponent& inventory);
};
