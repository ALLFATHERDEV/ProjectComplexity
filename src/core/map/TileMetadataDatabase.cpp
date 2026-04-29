#include "TileMetadataDatabase.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

using json = nlohmann::json;

std::string TileMetadataDatabase::makeKey(const std::string& paletteName, int atlasX, int atlasY) {
    return paletteName + "|" + std::to_string(atlasX) + "|" + std::to_string(atlasY);
}

bool TileMetadataDatabase::loadFromFolder(const std::string& folderPath) {
    m_SurfaceTagsByTile.clear();

    if (!std::filesystem::exists(folderPath)) {
        LOG_WARN("Tile metadata folder does not exist: {}", folderPath);
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            LOG_WARN("Could not open tile metadata file {}", entry.path().string());
            continue;
        }

        json data;
        file >> data;

        const std::string paletteName = data.value("paletteName", "");
        if (paletteName.empty()) {
            LOG_WARN("Tile metadata file {} has no paletteName", entry.path().string());
            continue;
        }

        if (!data.contains("tiles") || !data["tiles"].is_array()) {
            continue;
        }

        for (const auto& tileData : data["tiles"]) {
            const int atlasX = tileData.value("atlasX", 0);
            const int atlasY = tileData.value("atlasY", 0);

            std::vector<std::string> surfaceTags;
            if (tileData.contains("surfaceTags") && tileData["surfaceTags"].is_array()) {
                for (const auto& tag : tileData["surfaceTags"]) {
                    if (tag.is_string()) {
                        surfaceTags.push_back(tag.get<std::string>());
                    }
                }
            }

            m_SurfaceTagsByTile[makeKey(paletteName, atlasX, atlasY)] = surfaceTags;
        }
    }

    LOG_INFO("Loaded {} tile metadata entries from {}", m_SurfaceTagsByTile.size(), folderPath);
    return true;
}

const std::vector<std::string>* TileMetadataDatabase::getSurfaceTags(const std::string& paletteName, int atlasX, int atlasY) const {
    const auto it = m_SurfaceTagsByTile.find(makeKey(paletteName, atlasX, atlasY));
    if (it == m_SurfaceTagsByTile.end()) {
        return nullptr;
    }

    return &it->second;
}
