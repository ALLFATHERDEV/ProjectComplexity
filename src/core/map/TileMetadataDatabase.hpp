#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class OrePatchQuality {
    Impure,
    Normal,
    Pure,
    Perfect
};

struct TileMetadata {
    std::vector<std::string> surfaceTags;
    std::string minedItemName;
    OrePatchQuality orePatchQuality = OrePatchQuality::Normal;
};

class TileMetadataDatabase {
public:
    bool loadFromFolder(const std::string& folderPath);
    const std::vector<std::string>* getSurfaceTags(const std::string& paletteName, int atlasX, int atlasY) const;
    const std::string* getMinedItemName(const std::string& paletteName, int atlasX, int atlasY) const;
    OrePatchQuality getOrePatchQuality(const std::string& paletteName, int atlasX, int atlasY) const;
    static const char* orePatchQualityToString(OrePatchQuality quality);
    static float orePatchQualityToSpeedMultiplier(OrePatchQuality quality);

private:
    static OrePatchQuality orePatchQualityFromString(const std::string& quality);
    static std::string makeKey(const std::string& paletteName, int atlasX, int atlasY);

    std::unordered_map<std::string, TileMetadata> m_MetadataByTile;
};
