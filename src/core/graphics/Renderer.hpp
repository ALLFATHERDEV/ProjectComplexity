#ifndef PROJECTCOMPLEXITY_RENDERER_H
#define PROJECTCOMPLEXITY_RENDERER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Sprite.hpp"
#include <string>
#include <unordered_map>
#include <vector>

struct CachedText {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
};

struct SpriteDrawCommand {
    SDL_Texture* texture = nullptr;
    SDL_FRect srcRect{};
    SDL_FRect destRect{};
    int sortLayer = 0;
    SDL_FColor color{1.0f, 1.0f, 1.0f, 1.0f};
};

struct SpriteGeometryBatch {
    SDL_Texture* texture = nullptr;
    int sortLayer = 0;
    std::vector<SDL_Vertex> vertices;
    std::vector<int> indices;
};

class Renderer {
public:

    void init(SDL_Window* window);
    void loadFont(const char* path, float size);
    SDL_Renderer* getSDLRenderer() const;
    void clearRenderer() const;

    void queueSprite(const Sprite& sprite, const SDL_FRect& destRect, int sortLayer = 0);
    void queueSpriteAlpha(const Sprite& sprite, const SDL_FRect& destRect, int sortLayer, SDL_FColor color);
    void flushSpriteQueue();
    void clearSpriteQueue();
    size_t getLastQueuedSpriteCount() const { return m_LastQueuedSpriteCount; }
    size_t getLastTextureSwitchCount() const { return m_LastTextureSwitchCount; }

    void draw(SDL_Texture *texture, SDL_FRect srcRect, SDL_FRect &destRect) const;
    void drawSprite(const Sprite &sprite, SDL_FRect& destRect);
    void drawSpriteAlpha(const Sprite& sprite, SDL_FRect& destRect, Uint8 alpha);
    void drawFilledRect(const SDL_FRect& rect, SDL_Color color) const;
    void drawRect(const SDL_FRect& rect, SDL_Color color) const;
    void drawText(const std::string& text, float x, float y, SDL_Color color);

private:
    SDL_Renderer* m_Renderer = nullptr;
    TTF_Font* m_Font = nullptr;
    std::unordered_map<std::string, CachedText> m_TextCache;
    std::unordered_map<SDL_Texture*, SDL_FPoint> m_TextureSizeCache;
    std::vector<SpriteDrawCommand> m_SpriteQueue;
    size_t m_LastQueuedSpriteCount = 0;
    size_t m_LastTextureSwitchCount = 0;

    std::string makeTextCacheKey(const std::string& text, SDL_Color color);
    SDL_FPoint getTextureSizeCached(SDL_Texture* texture);
    void appendSpriteQuadToBatch(SpriteGeometryBatch& batch, const SpriteDrawCommand& command);
};

#endif //PROJECTCOMPLEXITY_RENDERER_H
