#pragma once

#include <string>
#include <vector>

struct MachineDefinition {
    std::string uniqueName;
    std::string displayName;
    std::vector<std::string> availableRecipes;

    int inputWidth = 1;
    int inputHeight = 1;
    int outputWidth = 1;
    int outputHeight = 1;
};
