#pragma once

#include <string>
#include <vector>

#include "../entities/component/FluidPortComponent.hpp"

enum class MachineType {
    None,
    Crafting,
    Miner,
    FluidTank,
    FluidPump
};

struct MachineFluidPortDefinition {
    std::string slotName;
    FluidPortType type = FluidPortType::INPUT;
    Direction side = Direction::RIGHT;
    int localTileX = 0;
    int localTileY = 0;
    float capacity = 100.0f;
    float maxTransferPerSecond = 50.0f;
};

struct MachineDefinition {
    virtual ~MachineDefinition() = default;

    std::string uniqueName;
    std::string displayName;
    MachineType type = MachineType::None;
    std::vector<std::string> availableRecipes;
    std::vector<std::string> allowedPlacementTags;
    std::vector<MachineFluidPortDefinition> fluidPorts;
    std::string spritePaletteName = "Overworld";
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

struct FluidTankMachineDefinition : MachineDefinition {
    FluidTankMachineDefinition() {
        type = MachineType::FluidTank;
        requiresFuel = false;
    }

    float capacity = 1000.0f;
};

struct FluidPumpMachineDefinition : MachineDefinition {
    FluidPumpMachineDefinition() {
        type = MachineType::FluidPump;
        requiresFuel = false;
    }

    float outputPerSecond = 100.0f;
};
