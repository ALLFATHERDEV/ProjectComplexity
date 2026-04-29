#ifndef PROJECTCOMPLEXITY_TILEMAP_H
#define PROJECTCOMPLEXITY_TILEMAP_H

#include <vector>

#include "TileMapLayer.hpp"

class ChunkManager;

class TileMap {
public:
    void setCellSize(int cellWidth, int cellHeight);
    void addLayer(const TileMapLayer &layer);
    void render(Renderer* renderer, const Camera2D& camera, const ChunkManager& chunkManager);
    void setTile(int x, int y, const Sprite &sprite, int layer, int atlasX, int atlasY, bool isBlocking = false);
    bool canPlaceTileObject(int x, int y, int layer, int widthTiles, int heightTiles) const;
    bool setTileObject(int x, int y, const Sprite& sprite, int layer, int widthTiles, int heightTiles, bool isBlocking, const std::string& itemName);
    void clearTile(int x, int y, int layer);
    bool isTileBlocking(int x, int y, int layer) const;
    bool isWorldPositionBlocking(float worldX, float worldY) const;
    bool isRectColliding(const SDL_FRect& rect) const;
    void clear();
    std::vector<TileMapLayer> getLayers() const { return m_Layers; }

private:
    std::vector<TileMapLayer> m_Layers;
    int m_CellWidth = 32;
    int m_CellHeight = 32;
};

#endif //PROJECTCOMPLEXITY_TILEMAP_H
