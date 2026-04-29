#pragma once

#include <string>
#include <vector>

struct CraftingMachineComponent {
    std::vector<std::string> availableRecipes;

    std::string currentRecipeName;

    float progress = 0.0f;
    bool isCrafting = false;
};
