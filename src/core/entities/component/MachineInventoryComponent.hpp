#pragma once
#include "../../inventory/InventoryGrid.hpp"

struct MachineInventoryComponent {
    InventoryGrid fuelInventory;
    InventoryGrid inputInventory;
    InventoryGrid outputInventory;
};
