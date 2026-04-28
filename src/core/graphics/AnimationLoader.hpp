#pragma once
#include "SpriteAtlas.hpp"
#include "../world/AnimationLibrary.hpp"

class AnimationLoader {
public:
    static void createSpriteAtlas(Renderer* renderer);
    static void loadPlayerAnimations(AnimationLibrary& library);

private:
    static SpriteAtlas m_PlayerAtlas;
};
