#pragma once

struct FluidDefinition;

struct FluidVolume {
    const FluidDefinition* fluid = nullptr;
    float amount = 0.0f;
};
