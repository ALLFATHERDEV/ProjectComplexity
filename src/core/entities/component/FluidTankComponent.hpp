#pragma once
#include "../../fluid/FluidVolume.hpp"

struct FluidTankComponent {
    FluidVolume storage;
    float capacity = 1000.0f;
};
