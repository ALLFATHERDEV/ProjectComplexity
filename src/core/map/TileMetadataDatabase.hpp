#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct TileMetadata {
    std::vector<std::string> surfaceTags;
    std::string minedItemName;
};

class TileMetadataDatabase {
public:
    bool loadFromFolder(const std::string& folderPath);
    const std::vector<std::string>* getSurfaceTags(const std::string& paletteName, int atlasX, int atlasY) const;
    const std::string* getMinedItemName(const std::string& paletteName, int atlasX, int atlasY) const;

private:
    static std::string makeKey(const std::string& paletteName, int atlasX, int atlasY);

    std::unordered_map<std::string, TileMetadata> m_MetadataByTile;
};
