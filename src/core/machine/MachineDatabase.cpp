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
        } else {
            auto baseMachine = std::make_unique<MachineDefinition>();
            loadBaseMachineFields(*baseMachine, data, machineType);
            machine = std::move(baseMachine);
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

    return MachineType::None;
}
