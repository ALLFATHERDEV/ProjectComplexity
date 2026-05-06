#pragma once

#include <string>

#include "FluidPortComponent.hpp"
#include "../Entity.hpp"

struct MachineFluidPortLinkComponent {
    Entity machineEntity = 0;
    std::string slotName;
    FluidPortType type = FluidPortType::INPUT;
};
