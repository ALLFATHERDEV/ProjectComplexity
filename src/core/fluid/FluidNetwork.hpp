#pragma once
#include <vector>

#include "FluidVolume.hpp"
#include "../entities/Entity.hpp"

struct FluidNetwork {
    int id = -1;

    std::vector<Entity> pipes;
    std::vector<Entity> tanks;
    std::vector<Entity> ports;

    FluidVolume fluid;
    float totalCapacity = 0.0f;
};
