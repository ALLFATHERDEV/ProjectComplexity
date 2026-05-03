#pragma once
#include "../ComponentStorage.hpp"
#include "../component/AnimationControllerComponent.hpp"
#include "../component/CharacterStateComponent.hpp"
#include "../component/ConveyorBeltComponent.hpp"

class AnimationStateSystem {
public:
    static void update(ComponentStorage<CharacterStateComponent>& states,
                       ComponentStorage<ConveyorBeltComponent>& conveyorBelts,
                       ComponentStorage<AnimationControllerComponent>& controllers);

private:
    static AnimatedSprite* resolveAnimation(AnimationControllerComponent& controller);
    static const char* characterStateToAnimationStateName(CharacterState state);
};
