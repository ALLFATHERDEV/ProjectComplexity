#include "ItemDebugEditor.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include "imgui.h"
#include "../Logger.hpp"

std::string ItemDebugEditor::toLowerCopy(const std::string& value) {
    std::string lowered = value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lowered;
}

bool ItemDebugEditor::matchesSearch(const ItemDefinition& item, const char* searchBuffer) {
    if (!searchBuffer || searchBuffer[0] == '\0') {
        return true;
    }

    const std::string search = toLowerCopy(searchBuffer);
    const std::string uniqueName = toLowerCopy(item.uniqueName);
    const std::string displayName = toLowerCopy(item.displayName);

    return uniqueName.find(search) != std::string::npos ||
           displayName.find(search) != std::string::npos;
}

void ItemDebugEditor::setEnabled(bool enabled) {
    m_Enabled = enabled;
}

void ItemDebugEditor::renderImGui(World& world) {
    if (!m_Enabled) return;

    ImGui::Begin("Item Debug");

    ImGui::InputText("Search", m_SearchBuffer, sizeof(m_SearchBuffer));
    ImGui::Separator();

    SpriteAtlas& itemAtlas = world.getItemAtlas();
    SDL_Texture* texture = itemAtlas.getTexture();

    int id = 0;
    auto& items = world.getItemDatabase().getAllItems();
    for (const auto&[name, item] : items) {
        if (!matchesSearch(item, m_SearchBuffer)) {
            continue;
        }

        ImGui::PushID(id);

        UVCoords uvCoords = itemAtlas.getUVCoordsOfSprite(item.iconAtlasX, item.iconAtlasY);
        if (ImGui::ImageButton("##title", (ImTextureID) texture, ImVec2(48, 48), ImVec2(uvCoords.u0, uvCoords.v0), ImVec2(uvCoords.u1, uvCoords.v1))) {
            const auto& playerInv = world.getInventories().get(1);
            playerInv->inventory.addItem(&item, 1);
        }

        ImGui::SameLine();

        ImGui::Text("%s", item.uniqueName.c_str());

        ImGui::PopID();

        id++;
    }

    ImGui::End();
}

bool ItemDebugEditor::isEnabled() const {
    return m_Enabled;
}
