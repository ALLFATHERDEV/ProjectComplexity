#pragma once
#include "ItemStack.hpp"

struct InventorySlot {
    ItemStack stack;

    bool isEmpty() const { return stack.isEmpty(); }
};
