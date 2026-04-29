#include "ConveyorSystem.hpp"

constexpr float kTileSize = 32.0f;
constexpr float kBlockedProgress = 0.5f;

struct TileDelta {
    int x;
    int y;
};

TileDelta getDelta(Direction direction) {
    switch (direction) {
        case Direction::DOWN:
            return {0, 1};
        case Direction::UP:
            return {0, -1};
        case Direction::LEFT:
            return {-1, 0};
        case Direction::RIGHT:
        default:
            return {1, 0};
    }
}

bool isEntityAtTile(const PositionComponent* position, int tileX, int tileY) {
    if (!position) {
        return false;
    }

    const int entityTileX = static_cast<int>(position->position.x / kTileSize);
    const int entityTileY = static_cast<int>(position->position.y / kTileSize);
    return entityTileX == tileX && entityTileY == tileY;
}

InventorySlot* getFirstOccupiedSlot(InventoryGrid& inventory) {
    for (int y = 0; y < inventory.getHeight(); y++) {
        for (int x = 0; x < inventory.getWidth(); x++) {
            InventorySlot* slot = inventory.getSlot(x, y);
            if (slot && !slot->isEmpty()) {
                return slot;
            }
        }
    }

    return nullptr;
}

bool canAcceptSingleItem(InventoryGrid& inventory, const ItemDefinition* item) {
    if (!item) {
        return false;
    }

    for (const auto& slot : inventory.getSlots()) {
        if (slot.isEmpty()) {
            return true;
        }

        if (slot.stack.item == item && slot.stack.amount < item->maxStackSize) {
            return true;
        }
    }

    return false;
}

Entity findBeltAtTile(ComponentStorage<ConveyorBeltComponent>& belts,
                      ComponentStorage<PositionComponent>& positions,
                      int tileX,
                      int tileY) {
    for (Entity entity : belts.getEntities()) {
        if (isEntityAtTile(positions.get(entity), tileX, tileY)) {
            return entity;
        }
    }

    return 0;
}

Entity findMachineAtTile(ComponentStorage<MachineInventoryComponent>& machines,
                         ComponentStorage<PositionComponent>& positions,
                         int tileX,
                         int tileY) {
    for (Entity entity : machines.getEntities()) {
        if (isEntityAtTile(positions.get(entity), tileX, tileY)) {
            return entity;
        }
    }

    return 0;
}

bool isBeltItemOccupyingTile(ComponentStorage<ConveyorItemComponent>& beltItems,
                             int tileX,
                             int tileY,
                             Entity ignoredEntity = 0) {
    const auto& items = beltItems.getRaw();
    const auto& entities = beltItems.getEntities();

    for (size_t i = 0; i < items.size(); i++) {
        if (entities[i] == ignoredEntity) {
            continue;
        }

        if (items[i].tileX == tileX && items[i].tileY == tileY) {
            return true;
        }
    }

    return false;
}

void updateItemPosition(PositionComponent& position, const ConveyorItemComponent& item) {
    const TileDelta delta = getDelta(item.direction);
    const float clampedProgress = item.progress > 1.0f ? 1.0f : item.progress;

    position.position.x = static_cast<float>(item.tileX) * kTileSize + static_cast<float>(delta.x) * kTileSize * clampedProgress;
    position.position.y = static_cast<float>(item.tileY) * kTileSize + static_cast<float>(delta.y) * kTileSize * clampedProgress;
}

void destroyBeltItem(Entity entity,
                     ComponentStorage<PositionComponent>& positions,
                     ComponentStorage<SpriteComponent>& sprites,
                     ComponentStorage<ConveyorItemComponent>& beltItems) {
    positions.remove(entity);
    sprites.remove(entity);
    beltItems.remove(entity);
}

bool canItemMoveForward(const ConveyorItemComponent& item,
                        ComponentStorage<PositionComponent>& positions,
                        ComponentStorage<ConveyorBeltComponent>& belts,
                        ComponentStorage<ConveyorItemComponent>& beltItems,
                        ComponentStorage<MachineInventoryComponent>& machineInventories) {
    const TileDelta delta = getDelta(item.direction);
    const int nextTileX = item.tileX + delta.x;
    const int nextTileY = item.tileY + delta.y;

    Entity machineEntity = findMachineAtTile(machineInventories, positions, nextTileX, nextTileY);
    if (machineEntity != 0) {
        auto* machineInventory = machineInventories.get(machineEntity);
        if (machineInventory && canAcceptSingleItem(machineInventory->inputInventory, item.item)) {
            return true;
        }
    }

    Entity nextBeltEntity = findBeltAtTile(belts, positions, nextTileX, nextTileY);
    if (nextBeltEntity == 0) {
        return false;
    }

    if (isBeltItemOccupyingTile(beltItems, nextTileX, nextTileY)) {
        return false;
    }

    return belts.get(nextBeltEntity) != nullptr;
}

float clampBlockedProgress(float currentProgress, float deltaProgress) {
    const float nextProgress = currentProgress + deltaProgress;
    return nextProgress > kBlockedProgress ? kBlockedProgress : nextProgress;
}

void ConveyorSystem::update(float deltaTime,
                            EntityManager& entityManager,
                            ComponentStorage<PositionComponent>& positions,
                            ComponentStorage<SpriteComponent>& sprites,
                            ComponentStorage<ConveyorBeltComponent>& belts,
                            ComponentStorage<ConveyorItemComponent>& beltItems,
                            ComponentStorage<MachineInventoryComponent>& machineInventories) {
    for (Entity machineEntity : machineInventories.getEntities()) {
        auto* machinePosition = positions.get(machineEntity);
        auto* machineInventory = machineInventories.get(machineEntity);
        if (!machinePosition || !machineInventory) {
            continue;
        }

        InventorySlot* outputSlot = getFirstOccupiedSlot(machineInventory->outputInventory);
        if (!outputSlot || outputSlot->isEmpty()) {
            continue;
        }

        const int machineTileX = static_cast<int>(machinePosition->position.x / kTileSize);
        const int machineTileY = static_cast<int>(machinePosition->position.y / kTileSize);

        constexpr Direction candidateDirections[] = {
            Direction::RIGHT,
            Direction::DOWN,
            Direction::LEFT,
            Direction::UP
        };

        for (Direction direction : candidateDirections) {
            const TileDelta delta = getDelta(direction);
            const int targetTileX = machineTileX + delta.x;
            const int targetTileY = machineTileY + delta.y;

            Entity beltEntity = findBeltAtTile(belts, positions, targetTileX, targetTileY);
            if (beltEntity == 0) {
                continue;
            }

            auto* belt = belts.get(beltEntity);
            if (!belt || belt->direction != direction) {
                continue;
            }

            if (isBeltItemOccupyingTile(beltItems, targetTileX, targetTileY)) {
                continue;
            }

            Entity itemEntity = entityManager.createEntity();
            ConveyorItemComponent beltItem;
            beltItem.item = outputSlot->stack.item;
            beltItem.tileX = targetTileX;
            beltItem.tileY = targetTileY;
            beltItem.direction = belt->direction;

            PositionComponent itemPosition{};
            itemPosition.position = {
                static_cast<float>(targetTileX) * kTileSize,
                static_cast<float>(targetTileY) * kTileSize
            };

            positions.add(itemEntity, itemPosition);
            sprites.add(itemEntity, { beltItem.item->icon, 1, 24.0f, 24.0f });
            beltItems.add(itemEntity, beltItem);
            machineInventory->outputInventory.removeItem(beltItem.item, 1);
            break;
        }
    }

    size_t index = 0;
    while (index < beltItems.getRaw().size()) {
        auto& items = beltItems.getRaw();
        auto& entities = beltItems.getEntities();

        Entity entity = entities[index];
        ConveyorItemComponent& item = items[index];
        PositionComponent* position = positions.get(entity);
        if (!position || !item.item) {
            destroyBeltItem(entity, positions, sprites, beltItems);
            continue;
        }

        if (item.blocked) {
            if (!canItemMoveForward(item, positions, belts, beltItems, machineInventories)) {
                item.progress = kBlockedProgress;
                updateItemPosition(*position, item);
                index++;
                continue;
            }

            item.blocked = false;
        }

        const float deltaProgress = deltaTime * item.speed;
        if (!canItemMoveForward(item, positions, belts, beltItems, machineInventories) &&
            item.progress < kBlockedProgress) {
            item.progress = clampBlockedProgress(item.progress, deltaProgress);
            item.blocked = item.progress >= kBlockedProgress;
            updateItemPosition(*position, item);
            index++;
            continue;
        }

        item.progress += deltaProgress;
        bool wasRemoved = false;

        while (item.progress >= 1.0f) {
            const TileDelta delta = getDelta(item.direction);
            const int nextTileX = item.tileX + delta.x;
            const int nextTileY = item.tileY + delta.y;

            Entity machineEntity = findMachineAtTile(machineInventories, positions, nextTileX, nextTileY);
            if (machineEntity != 0) {
                auto* machineInventory = machineInventories.get(machineEntity);
                if (machineInventory && machineInventory->inputInventory.addItem(item.item, 1)) {
                    destroyBeltItem(entity, positions, sprites, beltItems);
                    wasRemoved = true;
                    break;
                }
            }

            Entity nextBeltEntity = findBeltAtTile(belts, positions, nextTileX, nextTileY);
            if (nextBeltEntity == 0 || isBeltItemOccupyingTile(beltItems, nextTileX, nextTileY, entity)) {
                item.progress = kBlockedProgress;
                item.blocked = true;
                break;
            }

            auto* nextBelt = belts.get(nextBeltEntity);
            if (!nextBelt) {
                item.progress = kBlockedProgress;
                item.blocked = true;
                break;
            }

            item.tileX = nextTileX;
            item.tileY = nextTileY;
            item.direction = nextBelt->direction;
            item.blocked = false;
            item.progress -= 1.0f;
        }

        if (wasRemoved) {
            continue;
        }

        updateItemPosition(*position, item);
        index++;
    }
}
