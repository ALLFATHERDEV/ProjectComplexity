#include "TileMapLayer.hpp"

#include "../Logger.hpp"
#include "../camera/Camera2D.hpp"
#include "../graphics/Renderer.hpp"
#include "../world/ChunkManager.hpp"

class Camera2D;

void TileMapLayer::createLayer(int width, int height, int cellWidth, int cellHeight) {
    m_Width = width;
    m_Height = height;
    m_CellWidth = cellWidth;
    m_CellHeight = cellHeight;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Tile tile;
            tile.id = y * width + x;
            tile.x = x;
            tile.y = y;
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
            Tile& tile = m_Tiles[y * m_Width + x];
            if (!tile.shouldRender) {
                continue;
            }

            SDL_FRect dest{
                (static_cast<float>(tile.x * m_CellWidth) - camera.getX()) * camera.getZoom(),
                (static_cast<float>(tile.y * m_CellHeight) - camera.getY()) * camera.getZoom(),
                static_cast<float>(m_CellWidth) * camera.getZoom(),
                static_cast<float>(m_CellHeight) * camera.getZoom()
            };
            renderer->drawSprite(tile.sprite, dest);
        }
    }
}

void TileMapLayer::setTile(int x, int y, const Sprite &sprite, int atlasX, int atlasY, bool isBlocking) {
    if (x < 0 || y < 0 || x >= m_Width || y >= m_Height) {
        LOG_WARN("Could not set tile on coords: {}/{}", x, y);
        return;
    }

    int index = y * m_Width + x;
    m_Tiles[index].x = x;
    m_Tiles[index].y = y;
    m_Tiles[index].atlasX = atlasX;
    m_Tiles[index].atlasY = atlasY;
    m_Tiles[index].isBlocking = isBlocking;
    m_Tiles[index].sprite = sprite;
    m_Tiles[index].shouldRender = true;
}

void TileMapLayer::clearTile(int x, int y) {
    if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
        return;

    int index = y * m_Width + x;
    m_Tiles[index].id = -1;
    m_Tiles[index].sprite = Sprite{};
    m_Tiles[index].shouldRender = false;
}

void TileMapLayer::fill(const Sprite &sprite, int atlasX, int atlasY) {
    for (int y = 0; y < m_Height; y++)
        for (int x = 0; x < m_Width; x++)
            setTile(x, y, sprite, atlasX, atlasY);
}

bool TileMapLayer::isTileBlocking(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
        return true;

    int index = y * m_Width + x;
    return m_Tiles[index].isBlocking;
}

bool TileMapLayer::isRectColliding(const SDL_FRect &rect) const {
    int startX = static_cast<int>(rect.x) / m_CellWidth;
    int startY = static_cast<int>(rect.y) / m_CellHeight;
    int endX = static_cast<int>(rect.x + rect.w - 1) / m_CellWidth;
    int endY = static_cast<int>(rect.y + rect.h - 1) / m_CellHeight;

    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            if (isTileBlocking(x, y))
                return true;
        }
    }
    return false;

}

void TileMapLayer::clear() {
    for (int y = 0; y < m_Height; y++)
        for (int x = 0; x < m_Width; x++)
            clearTile(x, y);
}

