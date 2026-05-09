#include "Renderer.hpp"

#include "../Logger.hpp"


void Renderer::init(SDL_Window *window) {
    m_Renderer = SDL_CreateRenderer(window, nullptr);
    if (!m_Renderer) {
        LOG_ERROR("Could not create renderer: {}", SDL_GetError());
        exit(1);
    }

    m_SpriteQueue.reserve(8192);
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

SDL_FPoint Renderer::getTextureSizeCached(SDL_Texture* texture) {
    const auto it = m_TextureSizeCache.find(texture);
    if (it != m_TextureSizeCache.end()) {
        return it->second;
    }

    float textureWidth = 1.0f;
    float textureHeight = 1.0f;
    SDL_GetTextureSize(texture, &textureWidth, &textureHeight);
    const SDL_FPoint size{textureWidth, textureHeight};
    m_TextureSizeCache.emplace(texture, size);
    return size;
}

void Renderer::appendSpriteQuadToBatch(SpriteGeometryBatch &batch, const SpriteDrawCommand &command) {
    const SDL_FPoint textureSize = getTextureSizeCached(command.texture);
    const float textureWidth = textureSize.x;
    const float textureHeight = textureSize.y;

    const float x = command.destRect.x;
    const float y = command.destRect.y;
    const float w = command.destRect.w;
    const float h = command.destRect.h;

    const float u0 = command.srcRect.x / textureWidth;
    const float v0 = command.srcRect.y / textureHeight;
    const float u1 = (command.srcRect.x + command.srcRect.w) / textureWidth;
    const float v1 = (command.srcRect.y + command.srcRect.h) / textureHeight;

    const int baseIndex = static_cast<int>(batch.vertices.size());

    batch.vertices.push_back({{x,     y},     command.color, {u0, v0}});
    batch.vertices.push_back({{x + w, y},     command.color, {u1, v0}});
    batch.vertices.push_back({{x + w, y + h}, command.color, {u1, v1}});
    batch.vertices.push_back({{x,     y + h}, command.color, {u0, v1}});

    batch.indices.push_back(baseIndex + 0);
    batch.indices.push_back(baseIndex + 1);
    batch.indices.push_back(baseIndex + 2);
    batch.indices.push_back(baseIndex + 0);
    batch.indices.push_back(baseIndex + 2);
    batch.indices.push_back(baseIndex + 3);

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

void Renderer::queueSprite(const Sprite &sprite, const SDL_FRect &destRect, int sortLayer) {
    m_SpriteQueue.push_back({ sprite.texture, sprite.srcRect, destRect, sortLayer, {1.0f, 1.0f, 1.0f, 1.0f} });
}

void Renderer::queueSpriteAlpha(const Sprite &sprite, const SDL_FRect &destRect, int sortLayer, SDL_FColor color) {
    m_SpriteQueue.push_back({ sprite.texture, sprite.srcRect, destRect, sortLayer, color });
}

void Renderer::flushSpriteQueue() {
    m_LastQueuedSpriteCount = m_SpriteQueue.size();
    m_LastTextureSwitchCount = 0;
    SDL_Texture* lastBatchTexture = nullptr;

    if (m_SpriteQueue.empty())
        return;

    SpriteGeometryBatch batch;
    auto flushBatch = [this](SpriteGeometryBatch& currentBatch) {
        if (!currentBatch.texture || currentBatch.vertices.empty() || currentBatch.indices.empty())
            return;

        SDL_RenderGeometry(m_Renderer, currentBatch.texture, currentBatch.vertices.data(), static_cast<int>(currentBatch.vertices.size()), currentBatch.indices.data(), static_cast<int>(currentBatch.indices.size()));

        currentBatch.vertices.clear();
        currentBatch.indices.clear();
    };

    for (const SpriteDrawCommand& command : m_SpriteQueue) {
        if (!command.texture) {
            LOG_WARN("Skipped SpriteDrawCommand, texture is null");
            continue;
        }

        const bool needsNewBatch = batch.texture == nullptr || batch.texture != command.texture || batch.sortLayer != command.sortLayer;

        if (needsNewBatch) {
            flushBatch(batch);

            if (command.texture != lastBatchTexture) {
                m_LastTextureSwitchCount++;
                lastBatchTexture = command.texture;
            }

            batch.texture = command.texture;
            batch.sortLayer = command.sortLayer;
        }

        appendSpriteQuadToBatch(batch, command);
    }

    flushBatch(batch);
    m_SpriteQueue.clear();
}

void Renderer::clearSpriteQueue() {
    m_SpriteQueue.clear();
}

void Renderer::draw(SDL_Texture *texture, SDL_FRect srcRect, SDL_FRect &destRect) const {
    SDL_RenderTexture(m_Renderer, texture, &srcRect, &destRect);
}

void Renderer::drawSprite(const Sprite &sprite, SDL_FRect& destRect) {
    queueSprite(sprite, destRect, 0);
}

void Renderer::drawSpriteAlpha(const Sprite& sprite, SDL_FRect& destRect, Uint8 alpha) {
    SDL_FColor color{1.0f, 1.0f, 1.0f, alpha / 255.0f};
    queueSpriteAlpha(sprite, destRect, 0, color);
}

void Renderer::drawFilledRect(const SDL_FRect &rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_Renderer, &rect);
}

void Renderer::drawRect(const SDL_FRect &rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderRect(m_Renderer, &rect);
}


