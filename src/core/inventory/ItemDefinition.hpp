#pragma once

#include <string>
#include "../graphics/Sprite.hpp"

struct ItemDefinition {
    int id = -1;

    std::string uniqueName;
    std::string displayName;

    int maxStackSize = 1;

    int iconAtlasX = 0;
    int iconAtlasY = 0;

    Sprite icon;
};