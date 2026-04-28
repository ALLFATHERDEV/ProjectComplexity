#include "AnimatedRenderSystem.hpp"

#include "../../camera/Camera2D.hpp"

void AnimatedRenderSystem::render(Renderer* renderer, const Camera2D& camera, ComponentStorage<PositionComponent>& positions, ComponentStorage<AnimationControllerComponent>& controllers) {
    auto& controllerArray = controllers.getRaw();
    auto& entities = controllers.getEntities();

    for (size_t i = 0; i < controllerArray.size(); i++) {
        Entity entity = entities[i];

        auto* position = positions.get(entity);
        if (!position) continue;

        auto& controller = controllerArray[i];

        if (!controller.currentAnimation) continue;

        SDL_FRect destRect;
        destRect.x = position->position.x - camera.getX();
        destRect.y = position->position.y - camera.getY();
        destRect.w = 32;
        destRect.h = 32;

        controller.currentAnimation->render(renderer, destRect);
    }
}
