#ifndef PROJECTCOMPLEXITY_TILE_H
#define PROJECTCOMPLEXITY_TILE_H
#include <string>

#include "../graphics/Sprite.hpp"

struct Tile {
    int id = -1;
    int x = 0;
    int y = 0;

    int atlasX = 0;
    int atlasY = 0;
    int widthTiles = 1;
    int heightTiles = 1;
    int rootX = 0;
    int rootY = 0;

    Sprite sprite{};
    std::string placeableItemName;

    bool isOccupied = false;
    bool isPlaceableRoot = false;
    bool shouldRender = false;
    bool isBlocking = false;
};

#endif //PROJECTCOMPLEXITY_TILE_H
