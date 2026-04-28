#pragma once
#include "../ComponentStorage.hpp"
#include "../component/CharacterStateComponent.hpp"
#include "../component/VelocityComponent.hpp"

class CharacterStateSystem {
public:
    void update(ComponentStorage<CharacterStateComponent>& states, ComponentStorage<VelocityComponent>& velocities);
};
