//
// Created by tobia on 4/26/2026.
//

#include "RenderSystem.hpp"

#include "../../Logger.hpp"


void RenderSystem::render(Renderer *renderer, const Camera2D& camera, ComponentStorage<PositionComponent> &positions, ComponentStorage<SpriteComponent> &sprites) {
    auto& spriteArray = sprites.getRaw();
    auto& entities = sprites.getEntities();

    for (size_t i = 0; i < spriteArray.size(); i++) {
        Entity e = entities[i];

        auto* pos = positions.get(e);
        if (!pos) continue;

        auto& sprite = spriteArray[i].sprite;

        SDL_FRect destRect;
        destRect.x = pos->position.x - camera.getX();
        destRect.y = pos->position.y - camera.getY();
        destRect.w = sprite.srcRect.w;
        destRect.h = sprite.srcRect.h;

        renderer->drawSprite(sprite, destRect);
    }
}
