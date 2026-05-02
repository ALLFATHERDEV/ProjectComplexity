#pragma once
#include "../ComponentStorage.hpp"
#include "../component/AnimationControllerComponent.hpp"

class AnimationSystem {
public:
    static void update(float deltaTime, ComponentStorage<AnimationControllerComponent>& controllers);
};

