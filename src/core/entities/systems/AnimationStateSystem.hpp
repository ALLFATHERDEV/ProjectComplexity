#pragma once
#include "../ComponentStorage.hpp"
#include "../component/AnimationControllerComponent.hpp"
#include "../component/CharacterStateComponent.hpp"

class AnimationStateSystem {
public:
    static void update(ComponentStorage<CharacterStateComponent>& states, ComponentStorage<AnimationControllerComponent>& controllers);
};
