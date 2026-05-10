#pragma once
#include <optional>

#include "GoapState.hpp"
#include "../../entities/component/robot/RobotBrainComponent.hpp"

struct GoapActionDefinition {
    GoapActionType type;
    int cost = 1;

    std::array<std::optional<bool>, static_cast<size_t>(GoapFact::COUNT)> preconditions{};
    std::array<std::optional<bool>, static_cast<size_t>(GoapFact::COUNT)> effects{};
};

inline bool matchesPreconditions(const GoapState& state, const GoapActionDefinition& action) {
    for (size_t i = 0; i < action.preconditions.size(); i++) {
        if (!action.preconditions[i].has_value())
            continue;

        if (state.facts[i] != action.preconditions[i].value())
            return false;
    }

    return true;
}

inline GoapState applyEffects(const GoapState& state, const GoapActionDefinition& action) {
    GoapState result = state;

    for (size_t i = 0; i < action.effects.size(); i++) {
        if (action.effects[i].has_value()) {
            result.facts[i] = action.effects[i].value();
        }
    }

    return result;
}
