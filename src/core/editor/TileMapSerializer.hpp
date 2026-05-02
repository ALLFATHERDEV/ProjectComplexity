#pragma once

#include <string>

#include "../world/World.hpp"

class TileMapSerializer {
public:
    static bool save(const World& world, const std::string& filePath);
    static bool load(World& world, const std::string& filePath);
};
