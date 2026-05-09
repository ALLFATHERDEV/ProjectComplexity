#include "TileAnimationDatabase.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

using json = nlohmann::json;

bool TileAnimationDatabase::loadFromFolder(const std::string &folderPath, const std::function<SpriteAtlas*(const std::string)> &paletteResolver) {
    m_AnimationsByTile.clear();

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
            LOG_WARN("Tile animation file {} has no paletteName", entry.path().string());
            continue;
        }
        SpriteAtlas* atlas = paletteResolver(paletteName);
        if (!atlas) {
            LOG_WARN("Could not find palette {}", paletteName);
            continue;
        }

        TileAnimationDefinition definition;
        if (data.contains("frames") && data["frames"].is_array()) {
            for (const auto& frame : data["frames"]) {
                int x = frame.value("x", 0);
                int y = frame.value("y", 0);
                definition.frames.push_back(atlas->getSprite(x, y));
            }
        }
        definition.frameTime = data.value("frameTime", 0.0f);
        m_AnimationsByTile[makeKey(paletteName, data.value("sourceTileX", 0), data.value("sourceTileY", 0))] = definition;
        LOG_INFO("Loaded tile animation for tile {} from {}", makeKey(paletteName, data.value("sourceTileX", 0), data.value("sourceTileY", 0)), entry.path().string());
    }

    return true;
}

const TileAnimationDefinition* TileAnimationDatabase::getAnimation(const std::string &paletteName, int atlasX, int atlasY) const {
    const auto it = m_AnimationsByTile.find(makeKey(paletteName, atlasX, atlasY));
    if (it == m_AnimationsByTile.end()) {
        return nullptr;
    }

    return &it->second;
}

void TileAnimationDatabase::update(float deltaTime) {
    for (auto& [key, definition] : m_AnimationsByTile) {
        if (definition.frames.size() <= 1 || definition.frameTime <= 0.0f) {
            definition.currentFrameIndex = 0;
            definition.elapsedTime = 0.0f;
            continue;
        }

        definition.elapsedTime += deltaTime;
        while (definition.elapsedTime >= definition.frameTime) {
            definition.elapsedTime -= definition.frameTime;
            definition.currentFrameIndex = (definition.currentFrameIndex + 1) % static_cast<int>(definition.frames.size());
        }
    }
}

std::string TileAnimationDatabase::makeKey(const std::string &paletteName, int atlasX, int atlasY) {
    return paletteName + "|" + std::to_string(atlasX) + "|" + std::to_string(atlasY);
}
