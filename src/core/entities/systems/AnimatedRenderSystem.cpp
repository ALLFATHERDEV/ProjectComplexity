#include "AnimatedRenderSystem.hpp"

#include "../../camera/Camera2D.hpp"
#include <algorithm>
#include <numeric>

void AnimatedRenderSystem::render(Renderer* renderer, const Camera2D& camera, const ChunkManager& chunkManager, ComponentStorage<PositionComponent>& positions, ComponentStorage<AnimationControllerComponent>& controllers) {
    auto& controllerArray = controllers.getRaw();
    auto& entities = controllers.getEntities();

    std::vector<size_t> drawOrder(controllerArray.size());
    std::iota(drawOrder.begin(), drawOrder.end(), 0);

    std::stable_sort(drawOrder.begin(), drawOrder.end(), [&](size_t a, size_t b) {
        return controllerArray[a].sortOrder < controllerArray[b].sortOrder;
    });

    for (size_t index : drawOrder) {
        Entity entity = entities[index];

        auto* position = positions.get(entity);
        if (!position) continue;
        if (!chunkManager.isWorldPositionLoaded(position->position.x, position->position.y, 32)) continue;

        auto& controller = controllerArray[index];

        if (!controller.enabled || !controller.currentAnimation) continue;

        const float zoom = camera.getZoom();
        AnimatedSprite* currentAnimation = controller.currentAnimation;
        const float renderWidth = controller.renderWidth > 0.0f ? controller.renderWidth : 32.0f;
        const float renderHeight = controller.renderHeight > 0.0f ? controller.renderHeight : 32.0f;
        const float offsetX = controller.centerInSourceRect ? (32.0f - renderWidth) * 0.5f : 0.0f;
        const float offsetY = controller.centerInSourceRect ? (32.0f - renderHeight) * 0.5f : 0.0f;
        SDL_FRect destRect;
        destRect.x = (position->position.x - camera.getX() + offsetX) * zoom;
        destRect.y = (position->position.y - camera.getY() + offsetY) * zoom;
        destRect.w = renderWidth * zoom;
        destRect.h = renderHeight * zoom;

        currentAnimation->render(renderer, destRect);
    }
}
