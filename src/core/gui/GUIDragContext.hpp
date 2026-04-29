#pragma once

#include  "../inventory/ItemStack.hpp"

struct GUIDragContext {
    bool isDragging = false;
    bool suppressWorldPlacementUntilMouseRelease = false;
    ItemStack draggedStack;

    void clear() {
        isDragging = false;
        suppressWorldPlacementUntilMouseRelease = false;
        draggedStack.clear();
    }
};
