#pragma once

#include  "../inventory/ItemStack.hpp"

struct GUIDragContext {
    bool isDragging = false;
    ItemStack draggedStack;

    void clear() {
        isDragging = false;
        draggedStack.clear();
    }
};