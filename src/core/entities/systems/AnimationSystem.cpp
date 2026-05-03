#include "AnimationSystem.hpp"

void AnimationSystem::update(float deltaTime, ComponentStorage<AnimationControllerComponent>& controllers) {
    const auto& controllerArray = controllers.getRaw();

    for (auto& controller : controllerArray) {
        if (controller.enabled && controller.currentAnimation) {
            controller.currentAnimation->update(deltaTime);
        }
    }
}
