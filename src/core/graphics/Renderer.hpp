#ifndef PROJECTCOMPLEXITY_RENDERER_H
#define PROJECTCOMPLEXITY_RENDERER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Sprite.hpp"
#include <string>
#include <unordered_map>

struct CachedText {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
};

class Renderer {
public:

    void init(SDL_Window* window);
    void loadFont(const char* path, float size);
    SDL_Renderer* getSDLRenderer() const;
    void clearRenderer() const;

    void draw(SDL_Texture *texture, SDL_FRect srcRect, SDL_FRect &destRect) const;
    void drawSprite(const Sprite &sprite, SDL_FRect& destRect) const;
    void drawSpriteAlpha(const Sprite& sprite, SDL_FRect& destRect, Uint8 alpha) const;
    void drawFilledRect(const SDL_FRect& rect, SDL_Color color) const;
    void drawRect(const SDL_FRect& rect, SDL_Color color) const;
    void drawText(const std::string& text, float x, float y, SDL_Color color);

private:
    SDL_Renderer* m_Renderer = nullptr;
    TTF_Font* m_Font = nullptr;
    std::unordered_map<std::string, CachedText> m_TextCache;

    std::string makeTextCacheKey(const std::string& text, SDL_Color color);
};

#endif //PROJECTCOMPLEXITY_RENDERER_H
