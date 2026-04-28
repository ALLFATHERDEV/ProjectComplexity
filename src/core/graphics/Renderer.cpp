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

    SDL_Surface* surface = TTF_RenderText_Blended(m_Font, text.c_str(), text.length(), color);
    if (!surface)
        return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    if (!texture) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_FRect destRect{x, y, static_cast<float>(surface->w), static_cast<float>(surface->h)};
    SDL_RenderTexture(m_Renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

SDL_Renderer *Renderer::getSDLRenderer() const {
    return m_Renderer;
}


void Renderer::clearRenderer() const {

    if (m_Font) {
        TTF_CloseFont(m_Font);
    }
    TTF_Quit();

    SDL_DestroyRenderer(m_Renderer);
}

void Renderer::draw(SDL_Texture *texture, SDL_FRect srcRect, SDL_FRect &destRect) const {
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(m_Renderer, texture, &srcRect, &destRect);
}

void Renderer::drawSprite(const Sprite &sprite, SDL_FRect& destRect) const {
    SDL_SetTextureScaleMode(sprite.texture, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(m_Renderer, sprite.texture, &sprite.srcRect, &destRect);
}

void Renderer::drawFilledRect(const SDL_FRect &rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_Renderer, &rect);
}

void Renderer::drawRect(const SDL_FRect &rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderRect(m_Renderer, &rect);
}


