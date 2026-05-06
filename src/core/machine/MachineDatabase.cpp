#include "MachineDatabase.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

using json = nlohmann::json;

void loadBaseMachineFields(MachineDefinition& machine, const json& data, MachineType machineType) {
    machine.uniqueName = data.value("uniqueName", "");
    machine.displayName = data.value("displayName", "");
    machine.type = machineType;
    machine.spritePaletteName = data.value("spritePaletteName", "Overworld");
    machine.spriteAtlasX = data.value("spriteAtlasX", 0);
    machine.spriteAtlasY = data.value("spriteAtlasY", 0);
    machine.widthTiles = std::max(1, data.value("widthTiles", 1));
    machine.heightTiles = std::max(1, data.value("heightTiles", 1));
    machine.requiresFuel = data.value("requiresFuel", true);
    machine.fuelWidth = std::max(1, data.value("fuelWidth", 1));
    machine.fuelHeight = std::max(1, data.value("fuelHeight", 1));

    if (data.contains("availableRecipes") && data["availableRecipes"].is_array()) {
        for (const auto& recipeName : data["availableRecipes"]) {
            if (recipeName.is_string()) {
                machine.availableRecipes.push_back(recipeName.get<std::string>());
            }
        }
    }

    if (data.contains("allowedPlacementTags") && data["allowedPlacementTags"].is_array()) {
        for (const auto& tag : data["allowedPlacementTags"]) {
            if (tag.is_string()) {
                machine.allowedPlacementTags.push_back(tag.get<std::string>());
            }
        }
    }
}

bool MachineDatabase::loadMachinesFromFolder(const std::string& folderPath) {
    m_Machines.clear();

    if (!std::filesystem::exists(folderPath)) {
        LOG_ERROR("Machine folder does not exist: {}", folderPath);
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (entry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            LOG_WARN("Could not open machine file {}", entry.path().string());
            continue;
        }

        json data;
        file >> data;

        const MachineType machineType = stringToMachineType(data.value("type", ""));

        std::unique_ptr<MachineDefinition> machine;
        if (machineType == MachineType::Miner) {
            auto minerMachine = std::make_unique<MinerMachineDefinition>();
            loadBaseMachineFields(*minerMachine, data, machineType);
            minerMachine->miningSpeed = data.value("miningSpeed", 1.0f);
            machine = std::move(minerMachine);
        } else if (machineType == MachineType::FluidTank) {
            auto fluidTank = std::make_unique<FluidTankMachineDefinition>();
            loadBaseMachineFields(*fluidTank, data, machineType);
            fluidTank->capacity = data.value("capacity", 1000.0f);
            machine = std::move(fluidTank);
        } else if (machineType == MachineType::FluidPump) {
            auto fluidPump = std::make_unique<FluidPumpMachineDefinition>();
            loadBaseMachineFields(*fluidPump, data, machineType);
            fluidPump->outputPerSecond = data.value("outputPerSecond", 100.0f);
            machine = std::move(fluidPump);
        } else {
            auto baseMachine = std::make_unique<MachineDefinition>();
            loadBaseMachineFields(*baseMachine, data, machineType);
            machine = std::move(baseMachine);
        }

        if (data.contains("fluidPorts") && data["fluidPorts"].is_array()) {
            for (const auto& portData : data["fluidPorts"]) {
                if (!portData.is_object()) {
                    continue;
                }

                MachineFluidPortDefinition portDefinition;
                portDefinition.slotName = portData.value("slotName", portData.value("slot_name", ""));
                portDefinition.type = stringToFluidPortType(portData.value("type", "input"));
                portDefinition.side = stringToDirection(portData.value("side", "right"));
                portDefinition.localTileX = portData.value("localTileX", portData.value("local_tile_x", 0));
                portDefinition.localTileY = portData.value("localTileY", portData.value("local_tile_y", 0));
                portDefinition.capacity = std::max(0.0f, portData.value("capacity", 100.0f));
                portDefinition.maxTransferPerSecond = std::max(0.0f, portData.value("maxTransferPerSecond", portData.value("max_transfer_per_second", 50.0f)));

                if (portDefinition.slotName.empty()) {
                    continue;
                }

                machine->fluidPorts.push_back(portDefinition);
            }
        }

        if (machine->uniqueName.empty()) {
            LOG_WARN("Machine has no unique name: {}, skipping machine", entry.path().string());
            continue;
        }

        if (machine->type == MachineType::None) {
            LOG_WARN("Machine {} has no type, skipping machine", machine->uniqueName);
            continue;
        }

        if (machine->type == MachineType::Miner) {
            const auto* minerMachine = dynamic_cast<const MinerMachineDefinition*>(machine.get());
            if (!minerMachine) {
                LOG_WARN("Machine {} is marked as miner but failed to load as MinerMachineDefinition", machine->uniqueName);
            }
        }

        if (m_Machines.contains(machine->uniqueName)) {
            LOG_WARN("Machine with unique name {} already exists, skipping machine", machine->uniqueName);
            continue;
        }

        m_Machines[machine->uniqueName] = std::move(machine);
    }

    LOG_INFO("Loaded {} machines from folder {}", m_Machines.size(), folderPath);
    return true;
}

const MachineDefinition* MachineDatabase::getMachine(const std::string& uniqueName) const {
    auto it = m_Machines.find(uniqueName);
    if (it == m_Machines.end()) {
        return nullptr;
    }

    return it->second.get();
}

MachineType MachineDatabase::stringToMachineType(const std::string& str) const {
    if (str == "crafting") {
        return MachineType::Crafting;
    }

    if (str == "miner")
        return MachineType::Miner;

    if (str == "fluid_tank") {
        return MachineType::FluidTank;
    }

    if (str == "fluid_pump") {
        return MachineType::FluidPump;
    }

    return MachineType::None;
}

FluidPortType MachineDatabase::stringToFluidPortType(const std::string& str) const {
    if (str == "output") {
        return FluidPortType::OUTPUT;
    }

    if (str == "bidirectional") {
        return FluidPortType::BIDIRECTIONAL;
    }

    return FluidPortType::INPUT;
}

Direction MachineDatabase::stringToDirection(const std::string& str) const {
    if (str == "up") {
        return Direction::UP;
    }

    if (str == "down") {
        return Direction::DOWN;
    }

    if (str == "left") {
        return Direction::LEFT;
    }

    return Direction::RIGHT;
}
