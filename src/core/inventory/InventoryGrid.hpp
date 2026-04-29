#pragma once

#include <vector>

#include "InventorySlot.hpp"

class InventoryGrid {
public:
    void create(int width, int height);
    bool resizePreserve(int width, int height);
    bool addItem(const ItemDefinition* item, int amount);
    bool removeItem(const ItemDefinition* item, int amount);
    int countItem(const ItemDefinition* item) const;
    InventorySlot* getSlot(int x, int y);

    std::vector<InventorySlot>& getSlots() { return m_Slots; }
    const std::vector<InventorySlot>& getSlots() const { return m_Slots; }
    int getWidth() const { return m_Width; }
    int getHeight() const { return m_Height; }


private:
    int m_Width = 0;
    int m_Height = 0;

    std::vector<InventorySlot> m_Slots;
};
