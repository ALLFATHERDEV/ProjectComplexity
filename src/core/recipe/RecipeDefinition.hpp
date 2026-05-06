#pragma once

#include <string>
#include <vector>

#include "RecipeFluidStack.hpp"
#include "RecipeIngredient.hpp"

struct RecipeDefinition {
    std::string uniqueName;
    std::string displayName;

    std::vector<RecipeIngredient> itemInputs;
    std::vector<RecipeIngredient> itemOutputs;

    std::vector<RecipeFluidStack> fluidInputs;
    std::vector<RecipeFluidStack> fluidOutputs;

    float craftTime = 1.0f;
};
