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
    float fuelValue = 0.0f;
    bool isPlaceable = false;
    std::string placeableTexturePath;
    int placeableWidthTiles = 1;
    int placeableHeightTiles = 1;
    bool placeableBlocking = true;
    int placeableLayer = 1;
    std::string placedMachineUniqueName;
    bool placesStorageContainer = false;
    int containerInventoryWidth = 4;
    int containerInventoryHeight = 4;

    Sprite icon;
    Sprite placeableSprite;
};
