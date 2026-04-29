#include "AnimatedRenderSystem.hpp"

#include "../../camera/Camera2D.hpp"

void AnimatedRenderSystem::render(Renderer* renderer, const Camera2D& camera, const ChunkManager& chunkManager, ComponentStorage<PositionComponent>& positions, ComponentStorage<AnimationControllerComponent>& controllers) {
    auto& controllerArray = controllers.getRaw();
    auto& entities = controllers.getEntities();

    for (size_t i = 0; i < controllerArray.size(); i++) {
        Entity entity = entities[i];

        auto* position = positions.get(entity);
        if (!position) continue;
        if (!chunkManager.isWorldPositionLoaded(position->position.x, position->position.y, 32)) continue;

        auto& controller = controllerArray[i];

        if (!controller.currentAnimation) continue;

        const float zoom = camera.getZoom();
        SDL_FRect destRect;
        destRect.x = (position->position.x - camera.getX()) * zoom;
        destRect.y = (position->position.y - camera.getY()) * zoom;
        destRect.w = 32.0f * zoom;
        destRect.h = 32.0f * zoom;

        controller.currentAnimation->render(renderer, destRect);
    }
}
