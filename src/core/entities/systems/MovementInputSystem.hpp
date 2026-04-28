#pragma once
#include "../ComponentStorage.hpp"
#include "../component/InputComponent.hpp"
#include "../component/VelocityComponent.hpp"

class MovementInputSystem {
public:
    void update(ComponentStorage<InputComponent>& inputs, ComponentStorage<VelocityComponent>& velocities);
};
