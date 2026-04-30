#pragma once

#include <string>

struct MinerComponent {
    std::string machineUniqueName;
    std::string currentMinedItemName;
    bool requiresFuel = true;
    float currentFuelCapacity = 0.0f;
    float fuelRemaining = 0.0f;
    float miningProgress = 0.0f;
    float miningSpeed = 1.0f;
    int widthTiles = 1;
    int heightTiles = 1;
    bool isMining = false;
};
