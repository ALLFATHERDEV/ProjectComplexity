#pragma once

#include "../ComponentStorage.hpp"
#include "../../fluid/FluidDatabase.hpp"
#include "../../recipe/RecipeDatabase.hpp"
#include "../component/CraftingMachineComponent.hpp"
#include "../component/FluidPipeComponent.hpp"
#include "../component/FluidPortComponent.hpp"
#include "../component/FluidTankComponent.hpp"
#include "../component/MachineFluidComponent.hpp"
#include "../component/MachineFluidPortLinkComponent.hpp"
#include "FluidSystem.hpp"

struct MachineFluidIOContext;

class MachineFluidIOSystem {
public:
    void update(float deltaTime, MachineFluidIOContext& context,  FluidContext& fluidContext, const RecipeDatabase& recipeDatabase, const FluidDatabase& fluidDatabase, FluidSystem& fluidSystem);

private:
    MachineFluidSlot* findSlot(MachineFluidComponent& machineFluid, const MachineFluidPortLinkComponent& link) const;
    bool tryGetAllowedInputFluid(const MachineFluidPortLinkComponent& link,
                                 ComponentStorage<CraftingMachineComponent>& craftingMachines,
                                 const RecipeDatabase& recipeDatabase,
                                 const FluidDatabase& fluidDatabase,
                                 const FluidDefinition*& allowedFluid) const;
};
