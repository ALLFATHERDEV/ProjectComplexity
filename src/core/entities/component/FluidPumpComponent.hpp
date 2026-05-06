#pragma once

#include "../../fluid/FluidDefinition.hpp"

struct FluidPumpComponent {
    const FluidDefinition* outputFluid = nullptr;
    float outputPerSecond = 50.0f;
};
