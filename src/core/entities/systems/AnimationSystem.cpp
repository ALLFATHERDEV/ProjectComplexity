#include "AnimationSystem.hpp"

#include <ranges>

void AnimationSystem::update(float deltaTime, ComponentStorage<AnimationControllerComponent>& controllers) {
    auto& controllerArray = controllers.getRaw();

    for (auto& controller : controllerArray) {
        if (controller.currentAnimation) {
            controller.currentAnimation->update(deltaTime);
        }
    }
}
