#pragma once

#include <string>
#include <vector>

struct MachineDefinition {
    std::string uniqueName;
    std::string displayName;
    std::vector<std::string> availableRecipes;
    std::vector<std::string> allowedPlacementTags;
    int spriteAtlasX = 0;
    int spriteAtlasY = 0;
    int widthTiles = 1;
    int heightTiles = 1;

    bool requiresFuel = true;
    int fuelWidth = 1;
    int fuelHeight = 1;
};
