#include "GoapPlanner.hpp"

bool GoapPlanner::buildPlan(const GoapState &startState, const GoapState &goalState, std::vector<GoapActionType> &outPlan) const {
    outPlan.clear();

    const auto actions = createDefaultActions();

    std::vector<PlannerNode> open;
    std::vector<GoapState> visited;

    open.push_back({startState, {}});
    visited.push_back(startState);

    while (!open.empty()) {
        PlannerNode current = open.front();
        open.erase(open.begin());
        if (matchesGoal(current.state, goalState)) {
            outPlan = current.plan;
            return true;
        }

        for (const GoapActionDefinition& action : actions) {
            if (!matchesPreconditions(current.state, action))
                continue;;

            GoapState nextState = applyEffects(current.state, action);

            bool alreadyVisited = false;
            for (const GoapState& visitedState : visited) {
                if (visitedState == nextState) {
                    alreadyVisited = true;
                    break;
                }
            }

            if (alreadyVisited)
                continue;

            PlannerNode nextNode;
            nextNode.state = nextState;
            nextNode.plan = current.plan;
            nextNode.plan.push_back(action.type);

            open.push_back(nextNode);
            visited.push_back(nextState);
        }
    }

    return false;
}

std::vector<GoapActionDefinition> GoapPlanner::createDefaultActions() const {
    std::vector<GoapActionDefinition> actions;
    {
        GoapActionDefinition action;
        action.type = GoapActionType::MOVE_TO_PICKUP;
        action.cost = 1;
        action.preconditions[static_cast<size_t>(GoapFact::HAS_TASK)] = true;
        action.preconditions[static_cast<size_t>(GoapFact::CARRYING_ITEM)] = false;
        action.effects[static_cast<size_t>(GoapFact::AT_PICKUP)] = true;
        action.effects[static_cast<size_t>(GoapFact::AT_DROP_OFF)] = false;
        actions.push_back(action);
    }

    {
        GoapActionDefinition action;
        action.type = GoapActionType::PICKUP_ITEM;
        action.cost = 1;
        action.preconditions[static_cast<size_t>(GoapFact::HAS_TASK)] = true;
        action.preconditions[static_cast<size_t>(GoapFact::AT_PICKUP)] = true;
        action.preconditions[static_cast<size_t>(GoapFact::PICKUP_HAS_ITEM)] = true;
        action.preconditions[static_cast<size_t>(GoapFact::CARRYING_ITEM)] = false;
        action.effects[static_cast<size_t>(GoapFact::CARRYING_ITEM)] = true;
        actions.push_back(action);
    }

    {
        GoapActionDefinition action;
        action.type = GoapActionType::MOVE_TO_DROPOFF;
        action.cost = 1;
        action.preconditions[static_cast<size_t>(GoapFact::HAS_TASK)] = true;
        action.preconditions[static_cast<size_t>(GoapFact::CARRYING_ITEM)] = true;
        action.effects[static_cast<size_t>(GoapFact::AT_DROP_OFF)] = true;
        action.effects[static_cast<size_t>(GoapFact::AT_PICKUP)] = false;
        actions.push_back(action);
    }

    {
        GoapActionDefinition action;
        action.type = GoapActionType::DROP_OFF_ITEM;
        action.cost = 1;
        action.preconditions[static_cast<size_t>(GoapFact::HAS_TASK)] = true;
        action.preconditions[static_cast<size_t>(GoapFact::AT_DROP_OFF)] = true;
        action.preconditions[static_cast<size_t>(GoapFact::CARRYING_ITEM)] = true;
        action.preconditions[static_cast<size_t>(GoapFact::DROP_OFF_HAS_SPACE)] = true;
        action.effects[static_cast<size_t>(GoapFact::CARRYING_ITEM)] = false;
        action.effects[static_cast<size_t>(GoapFact::ITEM_DELIVERED)] = true;
        actions.push_back(action);
    }

    return actions;
}

bool GoapPlanner::matchesGoal(const GoapState &state, const GoapState &goal) const {
    for (size_t i = 0; i < goal.facts.size(); i++) {
        if (goal.facts[i] && !state.facts[i])
            return false;
    }
    return true;
}
