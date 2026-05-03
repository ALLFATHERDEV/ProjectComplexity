#pragma once
#include "SpriteAtlas.hpp"
#include "../world/AnimationLibrary.hpp"

class AnimationLoader {
public:
    static void loadAnimations(Renderer* renderer, AnimationLibrary& library);
    static void createSpriteAtlas(Renderer* renderer);

private:
    static std::unordered_map<std::string, SpriteAtlas*> m_AtlasMap;
    static SpriteAtlas m_PlayerAtlas;
    static SpriteAtlas m_ConveyerAtlas;
};
