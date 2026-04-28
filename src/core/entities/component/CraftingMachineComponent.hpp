#pragma once

#include <string>

struct CraftingMachineComponent {
    std::string currentRecipeName;

    float progress = 0.0f;
    bool isCrafting = false;
};