#ifndef PROJECTCOMPLEXITY_SPRITEATLAS_H
#define PROJECTCOMPLEXITY_SPRITEATLAS_H

#include <vector>

#include "Renderer.hpp"
#include "Sprite.hpp"

struct UVCoords {
    float u0;
    float v0;
    float u1;
    float v1;
};

class SpriteAtlas {
public:
    void createAtlas(const Renderer* renderer, int spriteWidth, int spriteHeight, const char* fileName);
    Sprite getSprite(int x, int y) const;
    UVCoords getUVCoordsOfSprite(int x, int y) const;
    void clear();

    int getNumSpritesX() const { return m_NumSpritesX; }
    int getNumSpritesY() const { return m_NumSpritesY; }
    int getSpriteWidth() const { return m_SpriteWidth; }
    int getSpriteHeight() const { return m_SpriteHeight; }
    float getTextureWidth() const { return m_TextureWidth; }
    float getTextureHeight() const { return m_TextureHeight; }
    SDL_Texture* getTexture() const { return m_Texture; }

private:
    int m_AtlasWidth = 0;
    int m_AtlasHeight = 0;
    int m_NumSpritesX = 0;
    int m_NumSpritesY = 0;
    int m_SpriteWidth = 0;
    int m_SpriteHeight = 0;
    float m_TextureWidth = 0;
    float m_TextureHeight = 0;
    SDL_Texture* m_Texture = nullptr;
    std::vector<Sprite> m_Sprites;
};

#endif //PROJECTCOMPLEXITY_SPRITEATLAS_H