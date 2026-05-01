#pragma once

#include <cstdint>

class SpriteAtlas;
class TileMap;

class TileMapGenerator {
public:
    enum class BiomeType {
        Plains,
        Desert,
        Stone,
        Lava,
        Water
    };

    struct TileConfig {
        int atlasX = 0;
        int atlasY = 0;
        bool isBlocking = false;
    };

    struct Config {
        int width = 128;
        int height = 128;
        uint32_t seed = 1337;
        TileConfig plainsTile{4, 1, false};
        TileConfig desertTile{10, 1, false};
        TileConfig stoneTile{7, 1, false};
        TileConfig lavaTile{3, 8, true};
        TileConfig waterTile{2, 0, true};
        double waterNoiseScale = 28.0;
        int waterOctaves = 4;
        double waterPersistence = 0.5;
        double lakeThreshold = -0.18;
        double biomeNoiseScale = 80.0;
        int biomeOctaves = 3;
        double biomePersistence = 0.5;
        double heatNoiseScale = 95.0;
        double lavaThreshold = 0.40;
        double stoneThreshold = 0.20;
        double desertHeatThreshold = 0.12;
    };

    static void generateTerrain(TileMap& tileMap, SpriteAtlas& atlas, const Config& config);
};
