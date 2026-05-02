#include "InventoryGrid.hpp"

void InventoryGrid::create(int width, int height) {
    m_Width = width;
    m_Height = height;
    m_Slots.clear();
    m_Slots.resize(width * height);
}

bool InventoryGrid::resizePreserve(int width, int height) {
    if (width == m_Width && height == m_Height)
        return true;

    const int oldWidth = m_Width;
    const int oldHeight = m_Height;
    const std::vector<InventorySlot> oldSlots = m_Slots;

    create(width, height);

    for (const auto& slot : oldSlots) {
        if (slot.isEmpty())
            continue;

        if (!addItem(slot.stack.item, slot.stack.amount)) {
            m_Width = oldWidth;
            m_Height = oldHeight;
            m_Slots = oldSlots;
            return false;
        }
    }

    return true;
}

bool InventoryGrid::addItem(const ItemDefinition *item, int amount) {
    if (!item || amount <= 0)
        return false;

    for (auto& slot : m_Slots) {
        if (slot.stack.item == item && slot.stack.amount < item->maxStackSize) {
            int space = item->maxStackSize - slot.stack.amount;
            int toAdd = std::min(space, amount);

            slot.stack.amount += toAdd;
            amount -= toAdd;

            if (amount <= 0)
                return true;
        }
    }

    for (auto& slot : m_Slots) {
        if (slot.isEmpty()) {
            int toAdd = std::min(item->maxStackSize, amount);
            slot.stack.item = item;
            slot.stack.amount = toAdd;

            amount -= toAdd;
            if (amount <= 0)
                return true;
        }
    }

    return false;
}

bool InventoryGrid::removeItem(const ItemDefinition *item, int amount) {
    if (!item || amount <= 0)
        return false;

    if (countItem(item) < amount)
        return false;

    for (auto& slot : m_Slots) {
        if (slot.stack.item != item)
            continue;

        int toRemove = std::min(slot.stack.amount, amount);
        slot.stack.amount -= toRemove;
        amount -= toRemove;

        if (slot.stack.amount <= 0)
            slot.stack.clear();

        if (amount <= 0)
            return true;
    }
    return true;
}

int InventoryGrid::countItem(const ItemDefinition *item) const {
    int count = 0;

    for (const auto& slot : m_Slots) {
        if (slot.stack.item == item)
            count += slot.stack.amount;
    }

    return count;
}


InventorySlot *InventoryGrid::getSlot(int x, int y) {
    if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
        return nullptr;

    return &m_Slots[y * m_Width + x];
}

