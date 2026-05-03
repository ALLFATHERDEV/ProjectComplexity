#include "SpriteAtlas.hpp"
#include "../Logger.hpp"
#include "SDL3_image/SDL_image.h"

void SpriteAtlas::createAtlas(const Renderer* renderer, int spriteWidth, int spriteHeight, const char* fileName) {
    m_Texture = IMG_LoadTexture(renderer->getSDLRenderer(), fileName);
    if (!m_Texture) {
        LOG_ERROR("Could not load texture: {}", SDL_GetError());
        return;
    }

    m_SpriteWidth = spriteWidth;
    m_SpriteHeight = spriteHeight;

    SDL_GetTextureSize(m_Texture, &m_TextureWidth, &m_TextureHeight);
    m_AtlasWidth = static_cast<int>(m_TextureWidth);
    m_AtlasHeight = static_cast<int>(m_TextureHeight);

    m_NumSpritesX = m_AtlasWidth / spriteWidth;
    m_NumSpritesY = m_AtlasHeight / spriteHeight;

    for (int y = 0; y < m_NumSpritesY; y++) {
        for (int x = 0; x < m_NumSpritesX; x++) {
            Sprite sprite;
            sprite.texture = m_Texture;
            sprite.srcRect = {static_cast<float>(x * spriteWidth), static_cast<float>(y * spriteHeight), static_cast<float>(spriteWidth), static_cast<float>(spriteHeight)};
            m_Sprites.push_back(sprite);
        }
    }
    // LOG_INFO("Created sprite atlas with {} sprites", m_NumSpritesX * m_NumSpritesY);

}

Sprite SpriteAtlas::getSprite(int x, int y) const {
    return m_Sprites[y * m_NumSpritesX + x];
}

UVCoords SpriteAtlas::getUVCoordsOfSprite(int x, int y) const {
    Sprite sprite = getSprite(x, y);
    return {
        sprite.srcRect.x / m_TextureWidth,
        sprite.srcRect.y / m_TextureHeight,
        (sprite.srcRect.x + sprite.srcRect.w) / m_TextureWidth,
        (sprite.srcRect.y + sprite.srcRect.h) / m_TextureHeight
    };
}

void SpriteAtlas::clear() {
    SDL_DestroyTexture(m_Texture);
    m_Texture = nullptr;
}
