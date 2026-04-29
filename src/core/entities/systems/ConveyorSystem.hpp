#pragma once

#include "../ComponentStorage.hpp"
#include "../EntityManager.hpp"
#include "../component/ConveyorBeltComponent.hpp"
#include "../component/ConveyorItemComponent.hpp"
#include "../component/MachineInventoryComponent.hpp"
#include "../component/PositionComponent.hpp"
#include "../component/SpriteComponent.hpp"
#include "../../inventory/ItemDatabase.hpp"

class ConveyorSystem {
public:
    void update(float deltaTime,
                EntityManager& entityManager,
                ComponentStorage<PositionComponent>& positions,
                ComponentStorage<SpriteComponent>& sprites,
                ComponentStorage<ConveyorBeltComponent>& belts,
                ComponentStorage<ConveyorItemComponent>& beltItems,
                ComponentStorage<MachineInventoryComponent>& machineInventories);
};
