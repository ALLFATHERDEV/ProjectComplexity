#pragma once
#include <vector>

#include "GoapAction.hpp"
#include "GoapState.hpp"
#include "../../entities/component/robot/RobotBrainComponent.hpp"

struct PlannerNode {
    GoapState state;
    std::vector<GoapActionType> plan;
};

class GoapPlanner {
public:
    bool buildPlan(const GoapState& startState, const GoapState& goalState, std::vector<GoapActionType>& outPlan) const;

private:
    std::vector<GoapActionDefinition> createDefaultActions() const;
    bool matchesGoal(const GoapState& state, const GoapState& goal) const;
};
