#include "ItemDatabase.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

using json = nlohmann::json;

bool ItemDatabase::loadItemsFromFolder(const std::string &folderPath, SpriteAtlas &atlas) {

    m_Items.clear();

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
        item.icon = atlas.getSprite(item.iconAtlasX, item.iconAtlasY);

        if (item.uniqueName.empty()) {
            LOG_WARN("Item has no unique name: {}, Skipping item", entry.path().string());
            continue;
        }
        if (m_Items.contains(item.uniqueName)) {
            LOG_WARN("Item with unique name {} already exists, Skipping item", item.uniqueName);
            continue;
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
