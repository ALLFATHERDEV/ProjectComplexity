#pragma once
#include "../ComponentStorage.hpp"
#include "../component/PositionComponent.hpp"
#include "../component/VelocityComponent.hpp"

class MovementSystem {
public:
    void update(float deltaTime, ComponentStorage<PositionComponent>& positions, ComponentStorage<VelocityComponent>& velocities);
};
