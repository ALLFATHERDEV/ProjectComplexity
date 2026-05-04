#pragma once
#include "CharacterStateComponent.hpp"

enum class FluidPortType {
    INPUT,
    OUTPUT,
    BIDIRECTIONAL
};

struct FluidPortComponent {
    FluidPortType type = FluidPortType::BIDIRECTIONAL;
    Direction side = Direction::RIGHT;
    float maxTransferPerSecond = 50.0f;
};
