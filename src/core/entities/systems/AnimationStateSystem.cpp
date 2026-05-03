#include "AnimationStateSystem.hpp"

const char* AnimationStateSystem::characterStateToAnimationStateName(CharacterState state) {
    switch (state) {
        case CharacterState::WALK:
            return "walk";
        case CharacterState::IDLE:
        default:
            return "idle";
    }
}

AnimatedSprite* AnimationStateSystem::resolveAnimation(AnimationControllerComponent& controller) {
    if (controller.useDirection) {
        const AnimationKey exactKey{controller.stateName, true, controller.direction};
        auto exactIt = controller.animations.find(exactKey);
        if (exactIt != controller.animations.end()) {
            return &exactIt->second;
        }
    }

    const AnimationKey stateOnlyKey{controller.stateName, false, Direction::DOWN};
    auto stateOnlyIt = controller.animations.find(stateOnlyKey);
    if (stateOnlyIt != controller.animations.end()) {
        return &stateOnlyIt->second;
    }

    if (controller.useDirection) {
        const AnimationKey defaultDirectionalKey{"default", true, controller.direction};
        auto defaultDirectionalIt = controller.animations.find(defaultDirectionalKey);
        if (defaultDirectionalIt != controller.animations.end()) {
            return &defaultDirectionalIt->second;
        }
    }

    const AnimationKey defaultKey{"default", false, Direction::DOWN};
    auto defaultIt = controller.animations.find(defaultKey);
    if (defaultIt != controller.animations.end()) {
        return &defaultIt->second;
    }

    return nullptr;
}

void AnimationStateSystem::update(ComponentStorage<CharacterStateComponent>& states,
                                  ComponentStorage<ConveyorBeltComponent>& conveyorBelts,
                                  ComponentStorage<AnimationControllerComponent>& controllers) {
    auto& controllerArray = controllers.getRaw();
    const auto& entities = controllers.getEntities();

    for (size_t i = 0; i < controllerArray.size(); i++) {
        const Entity entity = entities[i];

        auto& controller = controllerArray[i];

        if (const auto* state = states.get(entity)) {
            controller.stateName = characterStateToAnimationStateName(state->state);
            controller.useDirection = true;
            controller.direction = state->direction;
        } else if (const auto* belt = conveyorBelts.get(entity)) {
            controller.stateName = "default";
            controller.useDirection = true;
            controller.direction = belt->direction;
        }

        AnimatedSprite* nextAnimation = resolveAnimation(controller);
        if (!nextAnimation) {
            continue;
        }

        if (controller.currentAnimation != nextAnimation) {
            controller.currentAnimation = nextAnimation;
            controller.currentAnimation->reset();
            if (!controller.currentAnimation->isPlaying()) {
                controller.currentAnimation->play();
            }
        }
    }
}
