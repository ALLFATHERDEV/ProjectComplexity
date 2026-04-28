#include "AnimationStateSystem.hpp"

void AnimationStateSystem::update(ComponentStorage<CharacterStateComponent>& states, ComponentStorage<AnimationControllerComponent>& controllers) {
    auto& controllerArray = controllers.getRaw();
    auto& entities = controllers.getEntities();

    for (size_t i = 0; i < controllerArray.size(); i++) {
        Entity entity = entities[i];

        auto* state = states.get(entity);
        if (!state) continue;

        auto& controller = controllerArray[i];

        AnimationKey key {
            state->state,
            state->direction
        };

        auto it = controller.animations.find(key);
        if (it == controller.animations.end())
            continue;

        if (controller.currentAnimation != it->second) {
            controller.currentAnimation = it->second;
            controller.currentAnimation->reset();
            if (!controller.currentAnimation->isPlaying())
                controller.currentAnimation->play();

        }
    }
}