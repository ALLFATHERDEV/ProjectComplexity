#include "MachineDatabase.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

using json = nlohmann::json;

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

        MachineDefinition machine;
        machine.uniqueName = data.value("uniqueName", "");
        machine.displayName = data.value("displayName", "");
        machine.spriteAtlasX = data.value("spriteAtlasX", 0);
        machine.spriteAtlasY = data.value("spriteAtlasY", 0);
        machine.widthTiles = std::max(1, data.value("widthTiles", 1));
        machine.heightTiles = std::max(1, data.value("heightTiles", 1));
        machine.requiresFuel = data.value("requiresFuel", true);
        machine.fuelWidth = data.value("fuelWidth", 1);
        machine.fuelHeight = data.value("fuelHeight", 1);

        if (data.contains("availableRecipes") && data["availableRecipes"].is_array()) {
            for (const auto& recipeName : data["availableRecipes"]) {
                if (recipeName.is_string()) {
                    machine.availableRecipes.push_back(recipeName.get<std::string>());
                }
            }
        }

        if (machine.uniqueName.empty()) {
            LOG_WARN("Machine has no unique name: {}, skipping machine", entry.path().string());
            continue;
        }

        if (m_Machines.contains(machine.uniqueName)) {
            LOG_WARN("Machine with unique name {} already exists, skipping machine", machine.uniqueName);
            continue;
        }

        m_Machines[machine.uniqueName] = machine;
    }

    LOG_INFO("Loaded {} machines from folder {}", m_Machines.size(), folderPath);
    return true;
}

const MachineDefinition* MachineDatabase::getMachine(const std::string& uniqueName) const {
    auto it = m_Machines.find(uniqueName);
    if (it == m_Machines.end()) {
        return nullptr;
    }

    return &it->second;
}
