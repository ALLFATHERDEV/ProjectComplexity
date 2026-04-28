#pragma once
#include "../ComponentStorage.hpp"
#include "../component/InputComponent.hpp"
#include "../component/VelocityComponent.hpp"
#include "SDL3/SDL_events.h"

class InputSystem {
public:
    void handleInput(SDL_Event& event, ComponentStorage<InputComponent>& inputs, ComponentStorage<VelocityComponent>& velocities);
};
