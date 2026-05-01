#include "ItemDatabase.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <SDL3_image/SDL_image.h>

#include "../Logger.hpp"

using json = nlohmann::json;

ItemDatabase::~ItemDatabase() {
    for (auto& [_, texture] : m_PlaceableTextures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

bool ItemDatabase::loadItemsFromFolder(const std::string &folderPath, SpriteAtlas &atlas, const Renderer& renderer) {

    m_Items.clear();
    for (auto& [_, texture] : m_PlaceableTextures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    m_PlaceableTextures.clear();

    if (!std::filesystem::exists(folderPath)) {
        LOG_ERROR("Item folder does not exist: {}", folderPath);
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != ".json")
            continue;

        std::ifstream file(entry.path());
        if (!file.is_open())
            continue;

        json data;
        file >> data;

        ItemDefinition item;
        item.uniqueName = data.value("uniqueName", "");
        item.id = data.value("id", -1);
        item.displayName = data.value("displayName", "");
        item.maxStackSize = data.value("maxStackSize", 1);
        item.iconAtlasX = data.value("iconAtlasX", 0);
        item.iconAtlasY = data.value("iconAtlasY", 0);
        item.fuelValue = data.value("fuelValue", 0.0f);
        item.isPlaceable = data.value("isPlaceable", false);
        item.placeableTexturePath = data.value("placeableTexturePath", "");
        item.placeableWidthTiles = std::max(1, data.value("placeableWidthTiles", 1));
        item.placeableHeightTiles = std::max(1, data.value("placeableHeightTiles", 1));
        item.placeableBlocking = data.value("placeableBlocking", true);
        item.placeableLayer = data.value("placeableLayer", 1);
        item.placedMachineUniqueName = data.value("placedMachineUniqueName", "");
        item.placesStorageContainer = data.value("placesStorageContainer", false);
        item.containerInventoryWidth = std::max(1, data.value("containerInventoryWidth", 4));
        item.containerInventoryHeight = std::max(1, data.value("containerInventoryHeight", 4));
        item.icon = atlas.getSprite(item.iconAtlasX, item.iconAtlasY);

        if (item.uniqueName.empty()) {
            LOG_WARN("Item has no unique name: {}, Skipping item", entry.path().string());
            continue;
        }
        if (m_Items.contains(item.uniqueName)) {
            LOG_WARN("Item with unique name {} already exists, Skipping item", item.uniqueName);
            continue;
        }

        if (item.isPlaceable && !item.placeableTexturePath.empty()) {
            SDL_Texture* texture = IMG_LoadTexture(renderer.getSDLRenderer(), item.placeableTexturePath.c_str());
            if (!texture) {
                LOG_WARN("Could not load placeable texture {}", item.placeableTexturePath);
            } else {
                float textureWidth = 0.0f;
                float textureHeight = 0.0f;
                SDL_GetTextureSize(texture, &textureWidth, &textureHeight);
                item.placeableSprite.texture = texture;
                item.placeableSprite.srcRect = {0.0f, 0.0f, textureWidth, textureHeight};
                m_PlaceableTextures[item.uniqueName] = texture;
            }
        }

        m_Items[item.uniqueName] = item;
    }

    LOG_INFO("Loaded {} items from folder {}", m_Items.size(), folderPath);
    return true;
}

const ItemDefinition* ItemDatabase::getItem(const std::string &uniqueName) const {
    auto it = m_Items.find(uniqueName);
    if (it == m_Items.end())
        return nullptr;
    return &it->second;
}
