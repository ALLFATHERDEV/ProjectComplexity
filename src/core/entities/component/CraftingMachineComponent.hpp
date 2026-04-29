#pragma once

#include <string>
#include <vector>

struct CraftingMachineComponent {
    std::string machineUniqueName;
    std::vector<std::string> availableRecipes;

    std::string currentRecipeName;

    bool requiresFuel = true;
    float currentFuelCapacity = 0.0f;
    float progress = 0.0f;
    float fuelRemaining = 0.0f;
    bool isCrafting = false;
};
