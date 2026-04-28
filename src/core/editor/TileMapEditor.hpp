#pragma once
#include "../camera/Camera2D.hpp"
#include "../graphics/SpriteAtlas.hpp"
#include "../map/TileMap.hpp"
#include "SDL3/SDL_events.h"

class TileMapEditor {
public:
    void setEnabled(bool enabled);
    bool isEnabled() const;

    void update(SDL_Event& event, TileMap& tileMap, SpriteAtlas& tileAtlas, const Camera2D& camera);
    void renderImGui(TileMap& tileMap, SpriteAtlas& tileAtlas);
    void renderTilePalette(SpriteAtlas& atlas);

private:
    bool m_Enabled = false;

    int m_SelectedTileX = 0;
    int m_SelectedTileY = 0;
    int m_SelectedLayer = 0;
    bool m_SelectedBlocking = false;

    int m_cellWidth = 32;
    int m_CellHeight = 32;

};
