#pragma once

#include <string>
#include <vector>

enum class MachineType {
    None,
    Crafting,
    Miner
};

struct MachineDefinition {
    virtual ~MachineDefinition() = default;

    std::string uniqueName;
    std::string displayName;
    MachineType type = MachineType::None;
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

struct MinerMachineDefinition : MachineDefinition {
    MinerMachineDefinition() {
        type = MachineType::Miner;
    }

    float miningSpeed = 1.0f;
};
