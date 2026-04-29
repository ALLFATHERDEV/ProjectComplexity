#include "TileMap.hpp"
#include "../world/ChunkManager.hpp"


void TileMap::setCellSize(int cellWidth, int cellHeight) {
    m_CellWidth = cellWidth;
    m_CellHeight = cellHeight;
}

void TileMap::addLayer(const TileMapLayer &layer) {
    m_Layers.push_back(layer);
}

void TileMap::render(Renderer *renderer, const Camera2D& camera, const ChunkManager& chunkManager) {
    for (TileMapLayer& layer : m_Layers)
        layer.render(renderer, camera, chunkManager);
}

void TileMap::setTile(int x, int y, const Sprite &sprite, int layer, int atlasX, int atlasY, bool isBlocking) {
    TileMapLayer & mapLayer = m_Layers[layer];
    mapLayer.setTile(x, y, sprite, atlasX, atlasY, isBlocking);
}

bool TileMap::canPlaceTileObject(int x, int y, int layer, int widthTiles, int heightTiles) const {
    if (layer < 0 || layer >= static_cast<int>(m_Layers.size())) {
        return false;
    }

    return m_Layers[layer].canPlaceTileObject(x, y, widthTiles, heightTiles);
}

bool TileMap::setTileObject(int x, int y, const Sprite& sprite, int layer, int widthTiles, int heightTiles, bool isBlocking, const std::string& itemName) {
    if (layer < 0 || layer >= static_cast<int>(m_Layers.size())) {
        return false;
    }

    return m_Layers[layer].setTileObject(x, y, sprite, widthTiles, heightTiles, isBlocking, itemName);
}

void TileMap::clearTile(int x, int y, int layer) {
    TileMapLayer & mapLayer = m_Layers[layer];
    mapLayer.clearTile(x, y);
}

bool TileMap::isTileBlocking(int x, int y, int layer) const {
    if (layer < 0 || layer >= static_cast<int>(m_Layers.size()))
        return false;

    return m_Layers[layer].isTileBlocking(x, y);
}

bool TileMap::isWorldPositionBlocking(float worldX, float worldY) const {
    int tileX = static_cast<int>(worldX) / m_CellWidth;
    int tileY = static_cast<int>(worldY) / m_CellHeight;

    for (const auto& layer : m_Layers) {
        if (layer.isTileBlocking(tileX, tileY))
            return true;
    }

    return false;
}

bool TileMap::isRectColliding(const SDL_FRect &rect) const {
    for (const auto& layer : m_Layers) {
        if (layer.isRectColliding(rect))
            return true;
    }
    return false;
}

void TileMap::clear() {
    m_Layers.clear();
}
