#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include "ItemDefinition.hpp"
#include "../graphics/Renderer.hpp"
#include "../graphics/SpriteAtlas.hpp"

class ItemDatabase {
public:
    ~ItemDatabase();

    bool loadItemsFromFolder(const std::string& folderPath, SpriteAtlas& atlas, const Renderer& renderer);
    const ItemDefinition* getItem(const std::string& uniqueName) const;

private:
    std::unordered_map<std::string, ItemDefinition> m_Items;
    std::unordered_map<std::string, SDL_Texture*> m_PlaceableTextures;
};
