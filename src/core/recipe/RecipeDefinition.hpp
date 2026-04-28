#pragma once

#include <string>
#include <vector>
#include "RecipeIngredient.hpp"

struct RecipeDefinition {
    std::string uniqueName;
    std::string displayName;

    std::vector<RecipeIngredient> inputs;

    std::string outputItemName;
    int outputAmount = 1;

    float craftTime = 1.0f;
};