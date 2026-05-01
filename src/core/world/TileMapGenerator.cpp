#include "TileMapGenerator.hpp"

#include "../graphics/SpriteAtlas.hpp"
#include "../map/TileMap.hpp"
#include "../map/TileMapLayer.hpp"
#include "../util/PerlinNoise.hpp"

namespace {
const TileMapGenerator::TileConfig& getTileConfigForBiome(TileMapGenerator::BiomeType biome, const TileMapGenerator::Config& config) {
    switch (biome) {
        case TileMapGenerator::BiomeType::Desert:
            return config.desertTile;
        case TileMapGenerator::BiomeType::Stone:
            return config.stoneTile;
        case TileMapGenerator::BiomeType::Lava:
            return config.lavaTile;
        case TileMapGenerator::BiomeType::Water:
            return config.waterTile;
        case TileMapGenerator::BiomeType::Plains:
        default:
            return config.plainsTile;
    }
}

TileMapGenerator::BiomeType resolveBiome(const TileMapGenerator::Config& config, const PerlinNoise& waterNoise, const PerlinNoise& biomeNoise, const PerlinNoise& heatNoise, int x, int y) {
    const double waterValue = waterNoise.octaveNoise(
        static_cast<double>(x) / config.waterNoiseScale,
        static_cast<double>(y) / config.waterNoiseScale,
        config.waterOctaves,
        config.waterPersistence
    );

    if (waterValue < config.lakeThreshold) {
        return TileMapGenerator::BiomeType::Water;
    }

    const double biomeValue = biomeNoise.octaveNoise(
        static_cast<double>(x) / config.biomeNoiseScale,
        static_cast<double>(y) / config.biomeNoiseScale,
        config.biomeOctaves,
        config.biomePersistence
    );

    const double heatValue = heatNoise.octaveNoise(
        static_cast<double>(x) / config.heatNoiseScale,
        static_cast<double>(y) / config.heatNoiseScale,
        config.biomeOctaves,
        config.biomePersistence
    );

    if (biomeValue > config.lavaThreshold) {
        return TileMapGenerator::BiomeType::Lava;
    }

    if (biomeValue > config.stoneThreshold) {
        return TileMapGenerator::BiomeType::Stone;
    }

    if (heatValue > config.desertHeatThreshold) {
        return TileMapGenerator::BiomeType::Desert;
    }

    return TileMapGenerator::BiomeType::Plains;
}
}

void TileMapGenerator::generateTerrain(TileMap& tileMap, SpriteAtlas& atlas, const Config& config) {
    tileMap.clear();

    TileMapLayer groundLayer;
    groundLayer.createLayer(config.width, config.height, 32, 32);
    tileMap.addLayer(groundLayer);

    TileMapLayer objectLayer;
    objectLayer.createLayer(config.width, config.height, 32, 32);
    tileMap.addLayer(objectLayer);

    const PerlinNoise waterNoise(config.seed);
    const PerlinNoise biomeNoise(config.seed + 1);
    const PerlinNoise heatNoise(config.seed + 2);

    for (int y = 0; y < config.height; y++) {
        for (int x = 0; x < config.width; x++) {
            const BiomeType biome = resolveBiome(config, waterNoise, biomeNoise, heatNoise, x, y);
            const TileConfig& tileConfig = getTileConfigForBiome(biome, config);
            const Sprite sprite = atlas.getSprite(tileConfig.atlasX, tileConfig.atlasY);

            tileMap.setTile(
                x,
                y,
                sprite,
                0,
                "Overworld",
                tileConfig.atlasX,
                tileConfig.atlasY,
                tileConfig.isBlocking
            );
        }
    }
}
