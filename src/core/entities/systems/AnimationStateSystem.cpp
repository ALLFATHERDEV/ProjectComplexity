#include "AnimationStateSystem.hpp"

void AnimationStateSystem::update(ComponentStorage<CharacterStateComponent>& states, ComponentStorage<AnimationControllerComponent>& controllers) {
    auto& controllerArray = controllers.getRaw();
    const auto& entities = controllers.getEntities();

    for (size_t i = 0; i < controllerArray.size(); i++) {
        const Entity entity = entities[i];

        const auto* state = states.get(entity);
        if (!state) continue;

        auto&[animations, currentAnimation] = controllerArray[i];

        AnimationKey key {
            state->state,
            state->direction
        };

        auto it = animations.find(key);
        if (it == animations.end())
            continue;

        if (currentAnimation != it->second) {
            currentAnimation = it->second;
            currentAnimation->reset();
            if (!currentAnimation->isPlaying())
                currentAnimation->play();

        }
    }
}