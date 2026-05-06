#pragma once
#include <string>
#include <vector>

#include "../../fluid/FluidVolume.hpp"

struct MachineFluidSlot {
    std::string name;
    FluidVolume storage;
    float capacity = 0.0f;
};

struct MachineFluidComponent {
    std::vector<MachineFluidSlot> inputs;
    std::vector<MachineFluidSlot> outputs;
};
