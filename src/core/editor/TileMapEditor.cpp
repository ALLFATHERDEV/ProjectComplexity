#include "TileMapEditor.hpp"

#include "imgui.h"
#include "TileMapSerializer.hpp"
#include "../Logger.hpp"

void TileMapEditor::setEnabled(bool enabled) {
    m_Enabled = enabled;
}

bool TileMapEditor::isEnabled() const {
    return m_Enabled;
}

void TileMapEditor::update(SDL_Event &event, World& world, const Camera2D &camera) {
    if (!m_Enabled) return;

    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN)
        return;

    float mouseX;
    float mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    float worldX = mouseX + camera.getX();
    float worldY = mouseY + camera.getY();

    int tileX = static_cast<int>(worldX) / m_cellWidth;
    int tileY = static_cast<int>(worldY) / m_CellHeight;

    if (event.button.button == SDL_BUTTON_LEFT) {
        if (m_PlacementMode == PlacementMode::Tile) {
            Sprite sprite = world.getTileMapAtlas().getSprite(m_SelectedTileX, m_SelectedTileY);
            world.getTileMap().setTile(tileX, tileY, sprite, m_SelectedLayer, m_SelectedTileX, m_SelectedTileY, m_SelectedBlocking);
        } else {
            world.placeConveyorBelt(tileX, tileY, m_SelectedConveyorDirection);
        }
    }

    if (event.button.button == SDL_BUTTON_RIGHT) {
        if (m_PlacementMode == PlacementMode::Tile) {
            world.getTileMap().clearTile(tileX, tileY, m_SelectedLayer);
        } else {
            world.removeConveyorBelt(tileX, tileY);
        }
    }
}

void TileMapEditor::renderImGui(World& world) {
    if (!m_Enabled) return;

    if (m_PlacementMode == PlacementMode::Tile) {
        renderTilePalette(world.getTileMapAtlas());
    } else {
        renderConveyorPalette(world.getConveyorAtlas());
    }

    ImGui::Begin("TileMap Editor");

    int placementMode = static_cast<int>(m_PlacementMode);
    ImGui::RadioButton("Tiles", &placementMode, static_cast<int>(PlacementMode::Tile));
    ImGui::SameLine();
    ImGui::RadioButton("Conveyor", &placementMode, static_cast<int>(PlacementMode::Conveyor));
    m_PlacementMode = static_cast<PlacementMode>(placementMode);

    if (m_PlacementMode == PlacementMode::Tile) {
        ImGui::InputInt("Layer", &m_SelectedLayer);
        ImGui::Checkbox("Blocking", &m_SelectedBlocking);
    } else {
        int selectedDirection = static_cast<int>(m_SelectedConveyorDirection);
        ImGui::RadioButton("Right", &selectedDirection, static_cast<int>(Direction::RIGHT));
        ImGui::SameLine();
        ImGui::RadioButton("Down", &selectedDirection, static_cast<int>(Direction::DOWN));
        ImGui::SameLine();
        ImGui::RadioButton("Left", &selectedDirection, static_cast<int>(Direction::LEFT));
        ImGui::SameLine();
        ImGui::RadioButton("Up", &selectedDirection, static_cast<int>(Direction::UP));
        m_SelectedConveyorDirection = static_cast<Direction>(selectedDirection);
    }

    if (ImGui::Button("Save Map")) {
        TileMapSerializer::save(world, "maps/test.map");
    }

    if (ImGui::Button("Load Map")) {
        TileMapSerializer::load(world, "maps/test.map");
    }

    ImGui::End();
}

void TileMapEditor::renderTilePalette(SpriteAtlas &atlas) {

    ImGui::Begin("Tile Palette");

    const int columns = 8;
    const float tilePreviewSize = 48.0f;

    SDL_Texture* texture = atlas.getTexture();

    if (!texture) {
        ImGui::Text("No atlas texture loaded");
        ImGui::End();
        return;
    }

    for (int y = 0; y < columns; y++) {
        for (int x = 0; x < columns; x++) {
            ImGui::PushID(y * atlas.getNumSpritesX() + x);

            float u0 = static_cast<float>(x * atlas.getSpriteWidth()) / atlas.getTextureWidth();
            float v0 = static_cast<float>(y * atlas.getSpriteHeight()) / atlas.getTextureHeight();

            float u1 = static_cast<float>((x + 1) * atlas.getSpriteWidth()) / atlas.getTextureWidth();
            float v1 = static_cast<float>((y + 1) * atlas.getSpriteHeight()) / atlas.getTextureHeight();
            bool selected = m_SelectedTileX == x && m_SelectedTileY == y;

            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));

            if (ImGui::ImageButton("##title", (ImTextureID)texture, ImVec2(tilePreviewSize, tilePreviewSize), ImVec2(u0, v0), ImVec2(u1, v1))) {
                m_SelectedTileX = x;
                m_SelectedTileY = y;
            }

            if (selected)
                ImGui::PopStyleColor();

            if ((x + 1) % columns != 0)
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

void TileMapEditor::renderConveyorPalette(SpriteAtlas &atlas) {
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

    const ConveyorOption options[] = {
        {Direction::RIGHT, 3, 0, "Right"},
        {Direction::DOWN, 0, 0, "Down"},
        {Direction::LEFT, 2, 0, "Left"},
        {Direction::UP, 1, 0, "Up"}
    };

    constexpr float tilePreviewSize = 48.0f;
    for (int i = 0; i < 4; i++) {
        const ConveyorOption& option = options[i];
        ImGui::PushID(i);

        const float u0 = static_cast<float>(option.atlasX * atlas.getSpriteWidth()) / atlas.getTextureWidth();
        const float v0 = static_cast<float>(option.atlasY * atlas.getSpriteHeight()) / atlas.getTextureHeight();
        const float u1 = static_cast<float>((option.atlasX + 1) * atlas.getSpriteWidth()) / atlas.getTextureWidth();
        const float v1 = static_cast<float>((option.atlasY + 1) * atlas.getSpriteHeight()) / atlas.getTextureHeight();

        if (ImGui::ImageButton(option.label, (ImTextureID)texture, ImVec2(tilePreviewSize, tilePreviewSize), ImVec2(u0, v0), ImVec2(u1, v1))) {
            m_SelectedConveyorDirection = option.direction;
        }

        if (i < 3) {
            ImGui::SameLine();
        }

        ImGui::PopID();
    }

    ImGui::End();
}

