#pragma once
#include <string>

struct RobotComponent {
    std::string robotType;
    float moveSpeed = 50.0f;
    float interactionRange = 24.0f;
    int carryCapacity = 1;
};
