#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../graphics/Sprite.hpp"
#include "../graphics/SpriteAtlas.hpp"

struct TileAnimationDefinition {
    std::vector<Sprite> frames;
    float frameTime = 0.2f;

    float elapsedTime = 0.0f;
    int currentFrameIndex = 0;
};

class TileAnimationDatabase {
public:
    bool loadFromFolder(const std::string& folderPath, const std::function<SpriteAtlas*(const std::string)>& paletteResolver);
    const TileAnimationDefinition* getAnimation(const std::string& paletteName, int atlasX, int atlasY) const;
    void update(float deltaTime);

private:
    static std::string makeKey(const std::string& paletteName, int atlasX, int atlasY);

    std::unordered_map<std::string, TileAnimationDefinition> m_AnimationsByTile;
};
