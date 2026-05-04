#pragma once

#include "CharacterStateComponent.hpp"
#include "../../fluid/FluidVolume.hpp"

struct FluidPipeComponent {
    Direction direction = Direction::RIGHT;
    bool connectUp = false;
    bool connectDown = false;
    bool connectLeft = true;
    bool connectRight = true;
    FluidVolume storage;
    float capacity = 100.0f;
};
