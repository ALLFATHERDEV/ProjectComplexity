#ifndef PROJECTCOMPLEXITY_TILE_H
#define PROJECTCOMPLEXITY_TILE_H
#include "../graphics/Sprite.hpp"

struct Tile {
    int id = -1;
    int x = 0;
    int y = 0;

    int atlasX = 0;
    int atlasY = 0;

    Sprite sprite{};

    bool shouldRender = false;
    bool isBlocking = false;
};

#endif //PROJECTCOMPLEXITY_TILE_H