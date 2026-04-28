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

void TileMapEditor::update(SDL_Event &event, TileMap &tileMap, SpriteAtlas &tileAtlas, const Camera2D &camera) {
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
        Sprite sprite = tileAtlas.getSprite(m_SelectedTileX, m_SelectedTileY);
        tileMap.setTile(tileX, tileY, sprite, m_SelectedLayer, m_SelectedTileX, m_SelectedTileY, m_SelectedBlocking);
    }

    if (event.button.button == SDL_BUTTON_RIGHT) {
        tileMap.clearTile(tileX, tileY, m_SelectedLayer);
    }
}

void TileMapEditor::renderImGui(TileMap &tileMap, SpriteAtlas &tileAtlas) {
    if (!m_Enabled) return;

    renderTilePalette(tileAtlas);

    ImGui::Begin("TileMap Editor");

    ImGui::InputInt("Layer", &m_SelectedLayer);
    ImGui::Checkbox("Blocking", &m_SelectedBlocking);

    if (ImGui::Button("Save Map")) {
        TileMapSerializer::save(tileMap, "maps/test.map");
    }

    if (ImGui::Button("Load Map")) {
        TileMapSerializer::load(tileMap, tileAtlas, "maps/test.map");
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


