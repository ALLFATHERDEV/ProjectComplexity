#include "TileMetadataDatabase.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

using json = nlohmann::json;

OrePatchQuality TileMetadataDatabase::orePatchQualityFromString(const std::string& quality) {
    if (quality == "impure") {
        return OrePatchQuality::Impure;
    }

    if (quality == "pure") {
        return OrePatchQuality::Pure;
    }

    if (quality == "perfect") {
        return OrePatchQuality::Perfect;
    }

    return OrePatchQuality::Normal;
}

const char* TileMetadataDatabase::orePatchQualityToString(OrePatchQuality quality) {
    switch (quality) {
        case OrePatchQuality::Impure:
            return "Impure";
        case OrePatchQuality::Pure:
            return "Pure";
        case OrePatchQuality::Perfect:
            return "Perfect";
        case OrePatchQuality::Normal:
        default:
            return "Normal";
    }
}

float TileMetadataDatabase::orePatchQualityToSpeedMultiplier(OrePatchQuality quality) {
    switch (quality) {
        case OrePatchQuality::Impure:
            return 0.5f;
        case OrePatchQuality::Pure:
            return 1.5f;
        case OrePatchQuality::Perfect:
            return 2.0f;
        case OrePatchQuality::Normal:
        default:
            return 1.0f;
    }
}

std::string TileMetadataDatabase::makeKey(const std::string& paletteName, int atlasX, int atlasY) {
    return paletteName + "|" + std::to_string(atlasX) + "|" + std::to_string(atlasY);
}

bool TileMetadataDatabase::loadFromFolder(const std::string& folderPath) {
    m_MetadataByTile.clear();

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

            TileMetadata metadata;
            if (tileData.contains("surfaceTags") && tileData["surfaceTags"].is_array()) {
                for (const auto& tag : tileData["surfaceTags"]) {
                    if (tag.is_string()) {
                        metadata.surfaceTags.push_back(tag.get<std::string>());
                    }
                }
            }

            metadata.minedItemName = tileData.value("minedItemName", "");
            metadata.orePatchQuality = orePatchQualityFromString(tileData.value("orePatchQuality", "normal"));
            m_MetadataByTile[makeKey(paletteName, atlasX, atlasY)] = std::move(metadata);
        }
    }

    LOG_INFO("Loaded {} tile metadata entries from {}", m_MetadataByTile.size(), folderPath);
    return true;
}

const std::vector<std::string>* TileMetadataDatabase::getSurfaceTags(const std::string& paletteName, int atlasX, int atlasY) const {
    const auto it = m_MetadataByTile.find(makeKey(paletteName, atlasX, atlasY));
    if (it == m_MetadataByTile.end()) {
        return nullptr;
    }

    return &it->second.surfaceTags;
}

const std::string* TileMetadataDatabase::getMinedItemName(const std::string& paletteName, int atlasX, int atlasY) const {
    const auto it = m_MetadataByTile.find(makeKey(paletteName, atlasX, atlasY));
    if (it == m_MetadataByTile.end() || it->second.minedItemName.empty()) {
        return nullptr;
    }

    return &it->second.minedItemName;
}

OrePatchQuality TileMetadataDatabase::getOrePatchQuality(const std::string& paletteName, int atlasX, int atlasY) const {
    const auto it = m_MetadataByTile.find(makeKey(paletteName, atlasX, atlasY));
    if (it == m_MetadataByTile.end()) {
        return OrePatchQuality::Normal;
    }

    return it->second.orePatchQuality;
}
