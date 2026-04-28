#pragma once
#include "ItemDefinition.hpp"

struct ItemStack {
    const ItemDefinition* item = nullptr;
    int amount = 0;

    bool isEmpty() const { return item == nullptr || amount == 0; }

    void clear() {
        item = nullptr;
        amount = 0;
    }
};
