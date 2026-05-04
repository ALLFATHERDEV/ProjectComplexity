#include "TileMapEditor.hpp"

#include "imgui.h"
#include "TileMapSerializer.hpp"

void TileMapEditor::addTilePalette(const std::string& name, SpriteAtlas* atlas) {
    if (!atlas) {
        return;
    }

    m_TilePalettes.push_back({name, atlas});
}

void TileMapEditor::clearTilePalettes() {
    m_TilePalettes.clear();
    m_SelectedTilePaletteIndex = 0;
}

void TileMapEditor::setEnabled(const bool enabled) {
    m_Enabled = enabled;
}

bool TileMapEditor::isEnabled() const {
    return m_Enabled;
}

void TileMapEditor::update(const SDL_Event &event, World& world, const Camera2D &camera) {
    if (!m_Enabled) return;

    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN)
        return;

    float mouseX;
    float mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    const float worldX = mouseX / camera.getZoom() + camera.getX();
    const float worldY = mouseY / camera.getZoom() + camera.getY();

    const int tileX = static_cast<int>(worldX) / m_cellWidth;
    const int tileY = static_cast<int>(worldY) / m_CellHeight;

    if (event.button.button == SDL_BUTTON_LEFT) {
        if (m_PlacementMode == PlacementMode::Tile) {
            TilePaletteEntry* palette = getSelectedTilePalette();
            if (!palette || !palette->atlas) {
                return;
            }

            Sprite sprite = palette->atlas->getSprite(m_SelectedTileX, m_SelectedTileY);
            world.getTileMap().setTile(tileX, tileY, sprite, m_SelectedLayer, palette->name, m_SelectedTileX, m_SelectedTileY, m_SelectedBlocking);
        } else if (m_PlacementMode == PlacementMode::Conveyor) {
            world.placeConveyorBelt(tileX, tileY, m_SelectedConveyorDirection);
        } else if (m_PlacementMode == PlacementMode::FluidPipe) {
            world.placeFluidPipe(tileX, tileY, m_SelectedFluidPipeDirection);
        } else if (m_PlacementMode == PlacementMode::FluidTank) {
            world.placeFluidTank(tileX, tileY);
        } else if (m_PlacementMode == PlacementMode::FluidPump) {
            world.placeFluidPump(tileX, tileY, m_SelectedFluidPumpDirection);
        }
        // const TilePaletteEntry* palette = getSelectedTilePalette();
        // if (!palette || !palette->atlas) {
        //     return;
        // }
        //
        // const Sprite sprite = palette->atlas->getSprite(m_SelectedTileX, m_SelectedTileY);
        // world.getTileMap().setTile(tileX, tileY, sprite, m_SelectedLayer, palette->name, m_SelectedTileX, m_SelectedTileY, m_SelectedBlocking);
    }

    if (event.button.button == SDL_BUTTON_RIGHT) {
        if (m_PlacementMode == PlacementMode::Tile) {
            world.getTileMap().clearTile(tileX, tileY, m_SelectedLayer);
        } else if (m_PlacementMode == PlacementMode::Conveyor) {
            world.removeConveyorBelt(tileX, tileY);
        } else if (m_PlacementMode == PlacementMode::FluidPipe) {
            world.removeFluidPipe(tileX, tileY);
        } else if (m_PlacementMode == PlacementMode::FluidTank) {
            world.removeFluidTank(tileX, tileY);
        } else if (m_PlacementMode == PlacementMode::FluidPump) {
            world.removeFluidPump(tileX, tileY);
        }
        // world.getTileMap().clearTile(tileX, tileY, m_SelectedLayer);
    }
}

void TileMapEditor::renderImGui(World& world) {
    if (!m_Enabled) return;

    if (m_PlacementMode == PlacementMode::Tile) {
        renderTilePalette();
    } else {
        renderConveyorPalette(world.getConveyorAtlas());
    }

    ImGui::Begin("TileMap Editor");

    int placementMode = static_cast<int>(m_PlacementMode);
    ImGui::RadioButton("Tiles", &placementMode, static_cast<int>(PlacementMode::Tile));
    ImGui::SameLine();
    ImGui::RadioButton("Conveyor", &placementMode, static_cast<int>(PlacementMode::Conveyor));
    ImGui::SameLine();
    ImGui::RadioButton("Fluid Pipe", &placementMode, static_cast<int>(PlacementMode::FluidPipe));
    ImGui::SameLine();
    ImGui::RadioButton("Fluid Tank", &placementMode, static_cast<int>(PlacementMode::FluidTank));
    ImGui::SameLine();
    ImGui::RadioButton("Fluid Pump", &placementMode, static_cast<int>(PlacementMode::FluidPump));
    m_PlacementMode = static_cast<PlacementMode>(placementMode);

    if (m_PlacementMode == PlacementMode::Tile) {
        if (!m_TilePalettes.empty()) {
            std::vector<const char*> paletteNames;
            paletteNames.reserve(m_TilePalettes.size());
            for (const auto&[name, atlas] : m_TilePalettes) {
                paletteNames.push_back(name.c_str());
            }

            if (m_SelectedTilePaletteIndex < 0 || m_SelectedTilePaletteIndex >= static_cast<int>(m_TilePalettes.size())) {
                m_SelectedTilePaletteIndex = 0;
            }

            ImGui::Combo("Tile Palette", &m_SelectedTilePaletteIndex, paletteNames.data(), static_cast<int>(paletteNames.size()));
        }

        ImGui::InputInt("Layer", &m_SelectedLayer);
        ImGui::Checkbox("Blocking", &m_SelectedBlocking);
    } else if (m_PlacementMode == PlacementMode::Conveyor) {
        int selectedDirection = static_cast<int>(m_SelectedConveyorDirection);
        ImGui::RadioButton("Right", &selectedDirection, static_cast<int>(Direction::RIGHT));
        ImGui::SameLine();
        ImGui::RadioButton("Down", &selectedDirection, static_cast<int>(Direction::DOWN));
        ImGui::SameLine();
        ImGui::RadioButton("Left", &selectedDirection, static_cast<int>(Direction::LEFT));
        ImGui::SameLine();
        ImGui::RadioButton("Up", &selectedDirection, static_cast<int>(Direction::UP));
        m_SelectedConveyorDirection = static_cast<Direction>(selectedDirection);
    } else if (m_PlacementMode == PlacementMode::FluidPipe) {
        int selectedDirection = static_cast<int>(m_SelectedFluidPipeDirection);
        ImGui::RadioButton("Right", &selectedDirection, static_cast<int>(Direction::RIGHT));
        ImGui::SameLine();
        ImGui::RadioButton("Down", &selectedDirection, static_cast<int>(Direction::DOWN));
        ImGui::SameLine();
        ImGui::RadioButton("Left", &selectedDirection, static_cast<int>(Direction::LEFT));
        ImGui::SameLine();
        ImGui::RadioButton("Up", &selectedDirection, static_cast<int>(Direction::UP));
        m_SelectedFluidPipeDirection = static_cast<Direction>(selectedDirection);
        ImGui::Text("Left click: place pipe");
        ImGui::Text("Right click: remove pipe");
    } else if (m_PlacementMode == PlacementMode::FluidTank) {
        ImGui::Text("Left click: place tank");
        ImGui::Text("Right click: remove tank");
    } else if (m_PlacementMode == PlacementMode::FluidPump) {
        int selectedDirection = static_cast<int>(m_SelectedFluidPumpDirection);
        ImGui::RadioButton("Right", &selectedDirection, static_cast<int>(Direction::RIGHT));
        ImGui::SameLine();
        ImGui::RadioButton("Down", &selectedDirection, static_cast<int>(Direction::DOWN));
        ImGui::SameLine();
        ImGui::RadioButton("Left", &selectedDirection, static_cast<int>(Direction::LEFT));
        ImGui::SameLine();
        ImGui::RadioButton("Up", &selectedDirection, static_cast<int>(Direction::UP));
        m_SelectedFluidPumpDirection = static_cast<Direction>(selectedDirection);
    }

    if (ImGui::Button("Save Map")) {
        TileMapSerializer::save(world, "maps/test.map");
    }

    if (ImGui::Button("Load Map")) {
        TileMapSerializer::load(world, "maps/test.map");
    }

    ImGui::End();
}

void TileMapEditor::renderTilePalette() {
    const TilePaletteEntry* palette = getSelectedTilePalette();
    if (!palette || !palette->atlas) {
        return;
    }

    const SpriteAtlas& atlas = *palette->atlas;

    ImGui::Begin("Tile Palette");

    SDL_Texture* texture = atlas.getTexture();

    if (!texture) {
        ImGui::Text("No atlas texture loaded");
        ImGui::End();
        return;
    }

    const int columns = atlas.getNumSpritesX();
    const int rows = atlas.getNumSpritesY();

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            ImGui::PushID(y * atlas.getNumSpritesX() + x);

            const float u0 = static_cast<float>(x * atlas.getSpriteWidth()) / atlas.getTextureWidth();
            const float v0 = static_cast<float>(y * atlas.getSpriteHeight()) / atlas.getTextureHeight();

            const float u1 = static_cast<float>((x + 1) * atlas.getSpriteWidth()) / atlas.getTextureWidth();
            const float v1 = static_cast<float>((y + 1) * atlas.getSpriteHeight()) / atlas.getTextureHeight();
            const bool selected = m_SelectedTileX == x && m_SelectedTileY == y;

            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));

            if (constexpr float tilePreviewSize = 48.0f; ImGui::ImageButton("##title", (ImTextureID)texture, ImVec2(tilePreviewSize, tilePreviewSize), ImVec2(u0, v0), ImVec2(u1, v1))) {
                m_SelectedTileX = x;
                m_SelectedTileY = y;
            }

            if (selected)
                ImGui::PopStyleColor();

            if (x + 1 < columns)
                ImGui::SameLine();

            ImGui::PopID();
        }
    }

    ImGui::Separator();
    ImGui::Text("Selected: %d, %d", m_SelectedTileX, m_SelectedTileY);

    ImGui::Checkbox("Blocking", &m_SelectedBlocking);
    ImGui::InputInt("Layer", &m_SelectedLayer);

    ImGui::End();
}

TileMapEditor::TilePaletteEntry* TileMapEditor::getSelectedTilePalette() {
    if (m_SelectedTilePaletteIndex >= static_cast<int>(m_TilePalettes.size())) {
        return nullptr;
    }

    return &m_TilePalettes[m_SelectedTilePaletteIndex];
}

void TileMapEditor::renderConveyorPalette(const SpriteAtlas &atlas) {
    ImGui::Begin("Conveyor Palette");

    SDL_Texture* texture = atlas.getTexture();
    if (!texture) {
        ImGui::Text("No conveyor atlas loaded");
        ImGui::End();
        return;
    }

    struct ConveyorOption {
        Direction direction;
        int atlasX;
        int atlasY;
        const char* label;
    };

    constexpr ConveyorOption options[] = {
        {Direction::RIGHT, 0, 5, "Right"},
        {Direction::DOWN, 0, 6, "Down"},
        {Direction::LEFT, 7, 5, "Left"},
        {Direction::UP, 2, 6, "Up"}
    };

    constexpr float tilePreviewSize = 48.0f;
    for (int i = 0; i < 4; i++) {
        const auto&[direction, atlasX, atlasY, label] = options[i];
        ImGui::PushID(i);

        const float u0 = static_cast<float>(atlasX * atlas.getSpriteWidth()) / atlas.getTextureWidth();
        const float v0 = static_cast<float>(atlasY * atlas.getSpriteHeight()) / atlas.getTextureHeight();
        const float u1 = static_cast<float>((atlasX + 1) * atlas.getSpriteWidth()) / atlas.getTextureWidth();
        const float v1 = static_cast<float>((atlasY + 1) * atlas.getSpriteHeight()) / atlas.getTextureHeight();

        if (ImGui::ImageButton(label, (ImTextureID)texture, ImVec2(tilePreviewSize, tilePreviewSize), ImVec2(u0, v0), ImVec2(u1, v1))) {
            m_SelectedConveyorDirection = direction;
        }

        if (i < 3) {
            ImGui::SameLine();
        }

        ImGui::PopID();
    }

    ImGui::End();
}
