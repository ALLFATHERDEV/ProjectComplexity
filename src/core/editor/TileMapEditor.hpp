#pragma once
#include <string>
#include <vector>

#include "../camera/Camera2D.hpp"
#include "../entities/component/CharacterStateComponent.hpp"
#include "../graphics/SpriteAtlas.hpp"
#include "../world/World.hpp"
#include "SDL3/SDL_events.h"

class TileMapEditor {
public:
    struct TilePaletteEntry {
        std::string name;
        SpriteAtlas* atlas = nullptr;
    };

    enum class PlacementMode {
        Tile,
        Conveyor
    };

    void addTilePalette(const std::string& name, SpriteAtlas* atlas);
    void clearTilePalettes();
    void setEnabled(bool enabled);
    bool isEnabled() const;

    void update(SDL_Event& event, World& world, const Camera2D& camera);
    void renderImGui(World& world);
    void renderTilePalette();
    void renderConveyorPalette(SpriteAtlas& atlas);

private:
    TilePaletteEntry* getSelectedTilePalette();

    bool m_Enabled = false;
    PlacementMode m_PlacementMode = PlacementMode::Tile;
    std::vector<TilePaletteEntry> m_TilePalettes;
    int m_SelectedTilePaletteIndex = 0;

    int m_SelectedTileX = 0;
    int m_SelectedTileY = 0;
    int m_SelectedLayer = 0;
    bool m_SelectedBlocking = false;
    Direction m_SelectedConveyorDirection = Direction::RIGHT;

    int m_cellWidth = 32;
    int m_CellHeight = 32;

};
