#pragma once
#include <string>

#include "../entities/Entity.hpp"

enum class RobotTaskType {
    DELIVER_ITEM
};

enum class RobotTaskStatus {
    PENDING,
    RESERVED,
    COMPLETED,
    INVALID,
    BLOCKED
};

struct RobotTask {
    int id = -1;
    RobotTaskType type = RobotTaskType::DELIVER_ITEM;
    RobotTaskStatus status = RobotTaskStatus::PENDING;

    std::string itemName;
    int amount = 0;

    Entity pickupEntity = 0;
    Entity dropOffEntity = 0;

    Entity reservedBy = 0;
    std::string lastFailureReason;
};
