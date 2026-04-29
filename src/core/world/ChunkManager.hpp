#pragma once

#include "../util/MathUtil.hpp"

class ChunkManager {
public:
    void setChunkSizeTiles(int chunkSizeTiles);
    void setLoadRadius(int loadRadius);
    void update(const Vec2f& playerWorldPosition, int tileSize);

    bool isTileLoaded(int tileX, int tileY) const;
    bool isWorldPositionLoaded(float worldX, float worldY, int tileSize) const;

    int getLoadedTileMinX() const;
    int getLoadedTileMaxX() const;
    int getLoadedTileMinY() const;
    int getLoadedTileMaxY() const;

private:
    static int floorDiv(int value, int divisor);

    int m_ChunkSizeTiles = 32;
    int m_LoadRadius = 2;
    int m_CenterChunkX = 0;
    int m_CenterChunkY = 0;
};
