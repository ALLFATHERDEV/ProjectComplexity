#include "Renderer.hpp"

#include "../Logger.hpp"


void Renderer::init(SDL_Window *window) {
    m_Renderer = SDL_CreateRenderer(window, nullptr);
    if (!m_Renderer) {
        LOG_ERROR("Could not create renderer: {}", SDL_GetError());
        exit(1);
    }
}

void Renderer::loadFont(const char *path, float size) {
    if (!TTF_Init()) {
        LOG_ERROR("Failed to init SDL_ttf {}", SDL_GetError());
        return;
    }

    m_Font = TTF_OpenFont(path, size);
    if (!m_Font) {
        LOG_ERROR("Failed to load font: {}", SDL_GetError());
    }
}

void Renderer::drawText(const std::string &text, float x, float y, SDL_Color color) {
    if (!m_Font || text.empty())
        return;

    const std::string cacheKey = makeTextCacheKey(text, color);
    auto it = m_TextCache.find(cacheKey);
    if (it == m_TextCache.end()) {
        SDL_Surface* surface = TTF_RenderText_Blended(m_Font, text.c_str(), text.length(), color);
        if (!surface)
            return;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
        if (!texture) {
            SDL_DestroySurface(surface);
            return;
        }

        CachedText cachedText;
        cachedText.texture = texture;
        cachedText.width = surface->w;
        cachedText.height = surface->h;
        SDL_DestroySurface(surface);

        it = m_TextCache.emplace(cacheKey, cachedText).first;
    }


    SDL_FRect destRect{x, y, static_cast<float>(it->second.width), static_cast<float>(it->second.height)};
    SDL_RenderTexture(m_Renderer, it->second.texture, nullptr, &destRect);
}

std::string Renderer::makeTextCacheKey(const std::string &text, SDL_Color color) {
    return text + "|" +
               std::to_string(color.r) + "|" +
               std::to_string(color.g) + "|" +
               std::to_string(color.b) + "|" +
               std::to_string(color.a);
}

SDL_Renderer *Renderer::getSDLRenderer() const {
    return m_Renderer;
}


void Renderer::clearRenderer() const {

    for (auto& [key, cachedText] : m_TextCache) {
        if (cachedText.texture) {
            SDL_DestroyTexture(cachedText.texture);
        }
    }

    if (m_Font) {
        TTF_CloseFont(m_Font);
    }
    TTF_Quit();

    SDL_DestroyRenderer(m_Renderer);
}

void Renderer::draw(SDL_Texture *texture, SDL_FRect srcRect, SDL_FRect &destRect) const {
    SDL_RenderTexture(m_Renderer, texture, &srcRect, &destRect);
}

void Renderer::drawSprite(const Sprite &sprite, SDL_FRect& destRect) const {
    SDL_RenderTexture(m_Renderer, sprite.texture, &sprite.srcRect, &destRect);
}

void Renderer::drawSpriteAlpha(const Sprite& sprite, SDL_FRect& destRect, Uint8 alpha) const {
    SDL_SetTextureAlphaMod(sprite.texture, alpha);
    SDL_RenderTexture(m_Renderer, sprite.texture, &sprite.srcRect, &destRect);
    SDL_SetTextureAlphaMod(sprite.texture, 255);
}

void Renderer::drawFilledRect(const SDL_FRect &rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_Renderer, &rect);
}

void Renderer::drawRect(const SDL_FRect &rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderRect(m_Renderer, &rect);
}


