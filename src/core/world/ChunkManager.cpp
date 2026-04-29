#include "ChunkManager.hpp"

void ChunkManager::setChunkSizeTiles(int chunkSizeTiles) {
    if (chunkSizeTiles > 0) {
        m_ChunkSizeTiles = chunkSizeTiles;
    }
}

void ChunkManager::setLoadRadius(int loadRadius) {
    if (loadRadius >= 0) {
        m_LoadRadius = loadRadius;
    }
}

void ChunkManager::update(const Vec2f& playerWorldPosition, int tileSize) {
    const int tileX = static_cast<int>(playerWorldPosition.x) / tileSize;
    const int tileY = static_cast<int>(playerWorldPosition.y) / tileSize;

    m_CenterChunkX = floorDiv(tileX, m_ChunkSizeTiles);
    m_CenterChunkY = floorDiv(tileY, m_ChunkSizeTiles);
}

bool ChunkManager::isTileLoaded(int tileX, int tileY) const {
    const int chunkX = floorDiv(tileX, m_ChunkSizeTiles);
    const int chunkY = floorDiv(tileY, m_ChunkSizeTiles);

    return chunkX >= m_CenterChunkX - m_LoadRadius &&
           chunkX <= m_CenterChunkX + m_LoadRadius &&
           chunkY >= m_CenterChunkY - m_LoadRadius &&
           chunkY <= m_CenterChunkY + m_LoadRadius;
}

bool ChunkManager::isWorldPositionLoaded(float worldX, float worldY, int tileSize) const {
    const int tileX = static_cast<int>(worldX) / tileSize;
    const int tileY = static_cast<int>(worldY) / tileSize;
    return isTileLoaded(tileX, tileY);
}

int ChunkManager::getLoadedTileMinX() const {
    return (m_CenterChunkX - m_LoadRadius) * m_ChunkSizeTiles;
}

int ChunkManager::getLoadedTileMaxX() const {
    return (m_CenterChunkX + m_LoadRadius + 1) * m_ChunkSizeTiles - 1;
}

int ChunkManager::getLoadedTileMinY() const {
    return (m_CenterChunkY - m_LoadRadius) * m_ChunkSizeTiles;
}

int ChunkManager::getLoadedTileMaxY() const {
    return (m_CenterChunkY + m_LoadRadius + 1) * m_ChunkSizeTiles - 1;
}

int ChunkManager::floorDiv(int value, int divisor) {
    int quotient = value / divisor;
    int remainder = value % divisor;

    if (remainder != 0 && ((remainder < 0) != (divisor < 0))) {
        quotient--;
    }

    return quotient;
}
