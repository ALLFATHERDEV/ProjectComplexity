#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include "ItemDefinition.hpp"
#include "../graphics/SpriteAtlas.hpp"

class ItemDatabase {
public:
    bool loadItemsFromFolder(const std::string& folderPath, SpriteAtlas& atlas);
    const ItemDefinition* getItem(const std::string& uniqueName) const;

private:
    std::unordered_map<std::string, ItemDefinition> m_Items;
};