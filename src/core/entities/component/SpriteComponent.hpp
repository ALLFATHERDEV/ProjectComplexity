#pragma once
#include "../../graphics/Sprite.hpp"

struct SpriteComponent {
    Sprite sprite;
    int sortOrder = 0;
    float renderWidth = 0.0f;
    float renderHeight = 0.0f;
};
