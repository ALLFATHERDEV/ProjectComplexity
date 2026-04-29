#ifndef PROJECTCOMPLEXITY_TILEMAPLAYER_H
#define PROJECTCOMPLEXITY_TILEMAPLAYER_H

#include <vector>
#include "Tile.hpp"

class Camera2D;
class ChunkManager;
class Renderer;

class TileMapLayer {
public:
    void createLayer(int width, int height, int cellWidth, int cellHeight);
    void render(Renderer* renderer, const Camera2D& camera, const ChunkManager& chunkManager);
    void setTile(int x, int y, const Sprite &sprite, const std::string& paletteName, int atlasX, int atlasY, bool isBlocking = false);
    bool canPlaceTileObject(int x, int y, int widthTiles, int heightTiles) const;
    bool setTileObject(int x, int y, const Sprite& sprite, int widthTiles, int heightTiles, bool isBlocking, const std::string& itemName);
    void clearTile(int x, int y);
    void fill(const Sprite& sprite, const std::string& paletteName, int atlasX, int atlasY);
    bool isTileBlocking(int x, int y) const;
    bool isRectColliding(const SDL_FRect& rect) const;
    void clear();
    const Tile* getTile(int x, int y) const;
    int getWidth() const { return m_Width; }
    int getHeight() const { return m_Height; }
    int getCellWidth() const { return m_CellWidth; }
    int getCellHeight() const { return m_CellHeight; }
    const std::vector<Tile>& getTiles() const { return m_Tiles; }

private:
    int getTileIndex(int x, int y) const;
    bool isInBounds(int x, int y) const;
    void clearTileInternal(int x, int y);

    int m_Width = 0;
    int m_Height = 0;
    int m_CellWidth = 0;
    int m_CellHeight = 0;
    std::vector<Tile> m_Tiles;
};

#endif //PROJECTCOMPLEXITY_TILEMAPLAYER_H
