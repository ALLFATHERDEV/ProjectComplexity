#include "TileMapSerializer.hpp"
#include <fstream>
#include <sstream>

#include "../Logger.hpp"

bool TileMapSerializer::save(const TileMap &tileMap, const std::string &filePath) {
    std::ofstream file(filePath);

    if (!file.is_open()) {
        return false;
    }

    const auto& layers = tileMap.getLayers();

    file << "TILEMAP_V1\n";
    file << "LAYERS " << layers.size() << "\n";

    for (size_t layerIndex = 0; layerIndex < layers.size(); layerIndex++) {
        const TileMapLayer& layer = layers[layerIndex];

        file << "LAYER "
                << layerIndex << " "
                << layer.getWidth() << " "
                << layer.getHeight() << " "
                << layer.getCellWidth() << " "
                << layer.getCellHeight() << "\n";

        for (const Tile& tile : layer.getTiles()) {
            if (!tile.shouldRender) continue;

            file << "TILE "
                 << layerIndex << " "
                 << tile.x << " "
                 << tile.y << " "
                 << tile.atlasX << " "
                 << tile.atlasY << " "
                 << (tile.isBlocking ? 1 : 0)
                 << "\n";
        }
    }

    return true;
}

bool TileMapSerializer::load(TileMap &tileMap, SpriteAtlas &atlas, const std::string &filePath) {
    std::ifstream file(filePath);
    LOG_INFO("Loading map {}", filePath);

    if (!file.is_open())
        return false;

    tileMap.clear();

    std::string line;
    std::getline(file, line);

    if (line != "TILEMAP_V1")
        return false;


    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string type;
        ss >> type;

        if (type == "LAYERS")
        {
            // Kannst du erstmal ignorieren
        }
        else if (type == "LAYER")
        {
            int layerIndex;
            int width;
            int height;
            int cellWidth;
            int cellHeight;

            ss >> layerIndex >> width >> height >> cellWidth >> cellHeight;

            TileMapLayer layer;
            layer.createLayer(width, height, cellWidth, cellHeight);

            tileMap.addLayer(layer);
        }
        else if (type == "TILE")
        {
            int layerIndex;
            int x;
            int y;
            int atlasX;
            int atlasY;
            int blockingInt;

            ss >> layerIndex >> x >> y >> atlasX >> atlasY >> blockingInt;

            Sprite sprite = atlas.getSprite(atlasX, atlasY);

            tileMap.setTile(
                x,
                y,
                sprite,
                layerIndex,
                atlasX,
                atlasY,
                blockingInt != 0
            );
        }
    }

    LOG_INFO("Map loaded");

    return true;
}
