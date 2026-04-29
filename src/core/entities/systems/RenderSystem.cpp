#include "RenderSystem.hpp"

#include "../../Logger.hpp"
#include <algorithm>
#include <numeric>


void RenderSystem::render(Renderer *renderer, const Camera2D& camera, const ChunkManager& chunkManager, ComponentStorage<PositionComponent> &positions, ComponentStorage<SpriteComponent> &sprites) {
    auto& spriteArray = sprites.getRaw();
    auto& entities = sprites.getEntities();

    std::vector<size_t> drawOrder(spriteArray.size());
    std::iota(drawOrder.begin(), drawOrder.end(), 0);

    std::stable_sort(drawOrder.begin(), drawOrder.end(), [&](size_t a, size_t b) {
        return spriteArray[a].sortOrder < spriteArray[b].sortOrder;
    });

    for (size_t index : drawOrder) {
        Entity e = entities[index];

        auto* pos = positions.get(e);
        if (!pos) continue;
        if (!chunkManager.isWorldPositionLoaded(pos->position.x, pos->position.y, m_CellWidth)) continue;

        auto& sprite = spriteArray[index].sprite;

        const float renderWidth = spriteArray[index].renderWidth > 0.0f ? spriteArray[index].renderWidth : sprite.srcRect.w;
        const float renderHeight = spriteArray[index].renderHeight > 0.0f ? spriteArray[index].renderHeight : sprite.srcRect.h;
        const float zoom = camera.getZoom();
        const float offsetX = spriteArray[index].centerInSourceRect ? (sprite.srcRect.w - renderWidth) * 0.5f : 0.0f;
        const float offsetY = spriteArray[index].centerInSourceRect ? (sprite.srcRect.h - renderHeight) * 0.5f : 0.0f;

        SDL_FRect destRect;
        destRect.x = (pos->position.x - camera.getX() + offsetX) * zoom;
        destRect.y = (pos->position.y - camera.getY() + offsetY) * zoom;
        destRect.w = renderWidth * zoom;
        destRect.h = renderHeight * zoom;

        renderer->drawSprite(sprite, destRect);
    }
}
