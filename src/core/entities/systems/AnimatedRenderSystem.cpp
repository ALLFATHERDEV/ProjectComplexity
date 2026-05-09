#include "AnimatedRenderSystem.hpp"

#include "../../camera/Camera2D.hpp"
#include <algorithm>
#include <numeric>

namespace {
    constexpr int kAnimatedSpriteSortLayerBase = 1000;
}

std::vector<size_t> AnimatedRenderSystem::m_DrawOder;
size_t AnimatedRenderSystem::m_LastSpriteCount = 0;
bool AnimatedRenderSystem::m_DrawOrderDirty = 0;

void AnimatedRenderSystem::render(Renderer* renderer, const Camera2D& camera, const ChunkManager& chunkManager, ComponentStorage<PositionComponent>& positions, ComponentStorage<AnimationControllerComponent>& controllers) {
    auto& controllerArray = controllers.getRaw();
    auto& entities = controllers.getEntities();

    if (m_DrawOder.size() != controllerArray.size()) {
        m_DrawOder.resize(controllerArray.size());
    }
    std::iota(m_DrawOder.begin(), m_DrawOder.end(), 0);

    std::stable_sort(m_DrawOder.begin(), m_DrawOder.end(), [&](size_t a, size_t b) {
        return controllerArray[a].sortOrder < controllerArray[b].sortOrder;
    });

    for (size_t index : m_DrawOder) {
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

        currentAnimation->render(renderer, destRect, kAnimatedSpriteSortLayerBase + controller.sortOrder);
    }
}
