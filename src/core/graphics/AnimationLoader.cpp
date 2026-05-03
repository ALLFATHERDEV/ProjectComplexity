#include "AnimationLoader.hpp"

#include <Windows.UI.Composition.h>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

SpriteAtlas AnimationLoader::m_PlayerAtlas;
SpriteAtlas AnimationLoader::m_ConveyerAtlas;
std::unordered_map<std::string, SpriteAtlas*> AnimationLoader::m_AtlasMap;

using json = nlohmann::json;

void AnimationLoader::loadAnimations(Renderer* renderer, AnimationLibrary &library) {
    if (!std::filesystem::exists("assets/animations")) {
        LOG_ERROR("Animations folder does not exist");
        return;
    }


    int fileCounter = 0;
    for (const auto& entry : std::filesystem::directory_iterator("assets/animations")) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            LOG_WARN("Could not open animation file {}", entry.path().string());
            continue;
        }
        json data;
        file >> data;
        SpriteAtlas* atlas = m_AtlasMap[data.value("atlasTexturePath", "")];

        if (data.contains("animations") && data["animations"].is_array()) {
            for (const auto& animation : data["animations"]) {
                const std::string animationName = animation.value("name", "");
                std::vector<Sprite> frames;
                if (animation["frames"].is_array()) {
                    for (const auto& frame : animation["frames"]) {
                        // const auto& atlasCoords = frame["atlasCoords"];
                        const int atlasX = frame.value("x", 0);
                        const int atlasY = frame.value("y", 0);
                        frames.push_back(atlas->getSprite(atlasX, atlasY));
                    }
                }
                library.add(animationName, AnimatedSprite(frames, animation.value("loop", false), animation.value("playTime", 0.2f)));
            }
        }
        fileCounter++;
    }

    LOG_INFO("Loaded {} animations from folder {}", fileCounter, "assets/animations");

}

void AnimationLoader::createSpriteAtlas(Renderer* renderer) {
    m_PlayerAtlas.createAtlas(renderer, 32, 32, "assets/Sprite-0002.png");
    m_AtlasMap["assets/Sprite-0002.png"] = &m_PlayerAtlas;

    m_ConveyerAtlas.createAtlas(renderer, 32, 32, "assets/conveyor_sprites.png");
    m_AtlasMap["assets/conveyor_sprites.png"] = &m_ConveyerAtlas;
}


