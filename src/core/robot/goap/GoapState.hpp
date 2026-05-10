#pragma once
#include <array>

enum class GoapFact {
    HAS_TASK = 0,
    AT_PICKUP,
    PICKUP_HAS_ITEM,
    CARRYING_ITEM,
    AT_DROP_OFF,
    DROP_OFF_HAS_SPACE,
    ITEM_DELIVERED,
    COUNT
};

struct GoapState {
    std::array<bool, static_cast<size_t>(GoapFact::COUNT)> facts{};

    bool get(GoapFact fact) const {
        return facts[static_cast<size_t>(fact)];
    }

    void set(GoapFact fact, bool value) {
        facts[static_cast<size_t>(fact)] = value;
    }

    bool operator==(const GoapState& other) const {
        return facts == other.facts;
    }
};