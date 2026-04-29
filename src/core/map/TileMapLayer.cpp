#include "TileMapLayer.hpp"

#include <algorithm>

#include "../Logger.hpp"
#include "../camera/Camera2D.hpp"
#include "../graphics/Renderer.hpp"
#include "../world/ChunkManager.hpp"

int TileMapLayer::getTileIndex(int x, int y) const {
    return y * m_Width + x;
}

bool TileMapLayer::isInBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < m_Width && y < m_Height;
}

void TileMapLayer::createLayer(int width, int height, int cellWidth, int cellHeight) {
    m_Width = width;
    m_Height = height;
    m_CellWidth = cellWidth;
    m_CellHeight = cellHeight;
    m_Tiles.clear();
    m_Tiles.reserve(width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Tile tile;
            tile.id = y * width + x;
            tile.x = x;
            tile.y = y;
            tile.rootX = x;
            tile.rootY = y;
            m_Tiles.push_back(tile);
        }
    }
}

void TileMapLayer::render(Renderer* renderer, const Camera2D& camera, const ChunkManager& chunkManager) {
    const int startX = std::max(0, chunkManager.getLoadedTileMinX());
    const int startY = std::max(0, chunkManager.getLoadedTileMinY());
    const int endX = std::min(m_Width - 1, chunkManager.getLoadedTileMaxX());
    const int endY = std::min(m_Height - 1, chunkManager.getLoadedTileMaxY());

    if (endX < startX || endY < startY) {
        return;
    }

    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            Tile& tile = m_Tiles[getTileIndex(x, y)];
            if (!tile.shouldRender) {
                continue;
            }

            SDL_FRect dest{
                (static_cast<float>(tile.x * m_CellWidth) - camera.getX()) * camera.getZoom(),
                (static_cast<float>(tile.y * m_CellHeight) - camera.getY()) * camera.getZoom(),
                static_cast<float>(m_CellWidth * tile.widthTiles) * camera.getZoom(),
                static_cast<float>(m_CellHeight * tile.heightTiles) * camera.getZoom()
            };
            renderer->drawSprite(tile.sprite, dest);
        }
    }
}

void TileMapLayer::setTile(int x, int y, const Sprite &sprite, int atlasX, int atlasY, bool isBlocking) {
    if (!isInBounds(x, y)) {
        LOG_WARN("Could not set tile on coords: {}/{}", x, y);
        return;
    }

    clearTile(x, y);

    Tile& tile = m_Tiles[getTileIndex(x, y)];
    tile.id = getTileIndex(x, y);
    tile.x = x;
    tile.y = y;
    tile.atlasX = atlasX;
    tile.atlasY = atlasY;
    tile.widthTiles = 1;
    tile.heightTiles = 1;
    tile.rootX = x;
    tile.rootY = y;
    tile.sprite = sprite;
    tile.placeableItemName.clear();
    tile.isOccupied = true;
    tile.isPlaceableRoot = false;
    tile.shouldRender = true;
    tile.isBlocking = isBlocking;
}

bool TileMapLayer::canPlaceTileObject(int x, int y, int widthTiles, int heightTiles) const {
    for (int localY = 0; localY < heightTiles; localY++) {
        for (int localX = 0; localX < widthTiles; localX++) {
            const int checkX = x + localX;
            const int checkY = y + localY;
            if (!isInBounds(checkX, checkY)) {
                return false;
            }

            const Tile& tile = m_Tiles[getTileIndex(checkX, checkY)];
            if (tile.isOccupied) {
                return false;
            }
        }
    }

    return true;
}

bool TileMapLayer::setTileObject(int x, int y, const Sprite& sprite, int widthTiles, int heightTiles, bool isBlocking, const std::string& itemName) {
    if (!canPlaceTileObject(x, y, widthTiles, heightTiles)) {
        return false;
    }

    for (int localY = 0; localY < heightTiles; localY++) {
        for (int localX = 0; localX < widthTiles; localX++) {
            const int tileX = x + localX;
            const int tileY = y + localY;
            Tile& tile = m_Tiles[getTileIndex(tileX, tileY)];
            tile.id = getTileIndex(tileX, tileY);
            tile.x = tileX;
            tile.y = tileY;
            tile.atlasX = 0;
            tile.atlasY = 0;
            tile.widthTiles = 1;
            tile.heightTiles = 1;
            tile.rootX = x;
            tile.rootY = y;
            tile.sprite = Sprite{};
            tile.placeableItemName = itemName;
            tile.isOccupied = true;
            tile.isPlaceableRoot = false;
            tile.shouldRender = false;
            tile.isBlocking = isBlocking;
        }
    }

    Tile& root = m_Tiles[getTileIndex(x, y)];
    root.sprite = sprite;
    root.widthTiles = widthTiles;
    root.heightTiles = heightTiles;
    root.placeableItemName = itemName;
    root.shouldRender = true;
    root.isPlaceableRoot = true;
    return true;
}

void TileMapLayer::clearTile(int x, int y) {
    if (!isInBounds(x, y)) {
        return;
    }

    Tile& tile = m_Tiles[getTileIndex(x, y)];
    if (!tile.isOccupied && !tile.shouldRender) {
        clearTileInternal(x, y);
        return;
    }

    const int rootX = tile.isPlaceableRoot ? x : tile.rootX;
    const int rootY = tile.isPlaceableRoot ? y : tile.rootY;
    if (!isInBounds(rootX, rootY)) {
        clearTileInternal(x, y);
        return;
    }

    Tile& root = m_Tiles[getTileIndex(rootX, rootY)];
    if (!root.isPlaceableRoot) {
        clearTileInternal(x, y);
        return;
    }

    for (int localY = 0; localY < root.heightTiles; localY++) {
        for (int localX = 0; localX < root.widthTiles; localX++) {
            clearTileInternal(rootX + localX, rootY + localY);
        }
    }
}

void TileMapLayer::fill(const Sprite &sprite, int atlasX, int atlasY) {
    for (int y = 0; y < m_Height; y++) {
        for (int x = 0; x < m_Width; x++) {
            setTile(x, y, sprite, atlasX, atlasY);
        }
    }
}

bool TileMapLayer::isTileBlocking(int x, int y) const {
    if (!isInBounds(x, y)) {
        return true;
    }

    return m_Tiles[getTileIndex(x, y)].isBlocking;
}

bool TileMapLayer::isRectColliding(const SDL_FRect &rect) const {
    const int startX = static_cast<int>(rect.x) / m_CellWidth;
    const int startY = static_cast<int>(rect.y) / m_CellHeight;
    const int endX = static_cast<int>(rect.x + rect.w - 1) / m_CellWidth;
    const int endY = static_cast<int>(rect.y + rect.h - 1) / m_CellHeight;

    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            if (isTileBlocking(x, y)) {
                return true;
            }
        }
    }
    return false;
}

void TileMapLayer::clear() {
    for (int y = 0; y < m_Height; y++) {
        for (int x = 0; x < m_Width; x++) {
            clearTileInternal(x, y);
        }
    }
}

void TileMapLayer::clearTileInternal(int x, int y) {
    if (!isInBounds(x, y)) {
        return;
    }

    Tile& tile = m_Tiles[getTileIndex(x, y)];
    tile.id = -1;
    tile.x = x;
    tile.y = y;
    tile.atlasX = 0;
    tile.atlasY = 0;
    tile.widthTiles = 1;
    tile.heightTiles = 1;
    tile.rootX = x;
    tile.rootY = y;
    tile.sprite = Sprite{};
    tile.placeableItemName.clear();
    tile.isOccupied = false;
    tile.isPlaceableRoot = false;
    tile.shouldRender = false;
    tile.isBlocking = false;
}
