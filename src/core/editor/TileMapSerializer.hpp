#pragma once

#include <string>

#include "../graphics/SpriteAtlas.hpp"
#include "../map/TileMap.hpp"

class TileMapSerializer {
public:
    static bool save(const TileMap& tileMap, const std::string& filePath);
    static bool load(TileMap& tileMap, SpriteAtlas& atlas, const std::string& filePath);
};
