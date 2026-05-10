#pragma once
#include <string>
#include <vector>

#include "../../Entity.hpp"
#include "../../../util/MathUtil.hpp"

enum class GoapActionType {
    MOVE_TO_PICKUP,
    PICKUP_ITEM,
    MOVE_TO_DROPOFF,
    DROP_OFF_ITEM
};

struct RobotBrainComponent {
    int currentTaskId = -1;
    bool planDirty = true;
    std::string currentGoal;
    std::vector<GoapActionType> currentPlan;
    size_t currentPlanIndex = 0;

    Entity runtimeTargetEntity = 0;
    Vec2f runtimeMoveTarget{};
};