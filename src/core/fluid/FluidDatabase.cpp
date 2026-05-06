#include "FluidDatabase.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

using json = nlohmann::json;

bool FluidDatabase::loadFluidsFromFolder(const std::string &folderPath) {
    m_Fluids.clear();
    if (!std::filesystem::exists(folderPath)) {
        LOG_ERROR("Fluids folder does not exist: {}", folderPath);
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
            LOG_WARN("Could not open fluid file {}", entry.path().string());
            continue;
        }

        json data;
        file >> data;

        FluidDefinition fluid;
        fluid.uniqueName = data.value("uniqueName", "");
        fluid.displayName = data.value("displayName", "");

        if (fluid.uniqueName.empty()) {
            LOG_WARN("Fluid has no unique name: {}, skipping fluid", entry.path().string());
            continue;
        }

        m_Fluids[fluid.uniqueName] = fluid;
    }

    LOG_INFO("Loaded {} fluids from folder {}", m_Fluids.size(), folderPath);
    return true;
}

const FluidDefinition * FluidDatabase::getFluid(const std::string &uniqueName) const {
    auto it = m_Fluids.find(uniqueName);
    if (it == m_Fluids.end())
        return nullptr;
    return &it->second;
}
