#include "MiningSystem.hpp"

void MiningSystem::update(float deltaTime,
                          ComponentStorage<MinerComponent>& miners,
                          ComponentStorage<PositionComponent>& positions,
                          ComponentStorage<MachineInventoryComponent>& inventories,
                          const TileMap& tileMap,
                          const TileMetadataDatabase& tileMetadataDatabase,
                          const ItemDatabase& itemDatabase) {
    auto& minerArray = miners.getRaw();
    const auto& entities = miners.getEntities();

    for (size_t i = 0; i < minerArray.size(); i++) {
        const Entity entity = entities[i];
        PositionComponent* position = positions.get(entity);
        MachineInventoryComponent* inventory = inventories.get(entity);
        if (!position || !inventory) {
            continue;
        }

        MinerComponent& miner = minerArray[i];
        const MiningTarget target = resolveMiningTarget(miner, *position, tileMap, tileMetadataDatabase, itemDatabase);
        const ItemDefinition* minedItem = target.item;
        if (!minedItem) {
            miner.currentMinedItemName.clear();
            miner.currentOrePatchQuality = "Normal";
            miner.isMining = false;
            miner.miningProgress = 0.0f;
            continue;
        }
        miner.currentMinedItemName = minedItem->uniqueName;
        miner.currentOrePatchQuality = TileMetadataDatabase::orePatchQualityToString(target.quality);

        if (!canOutputFit(*inventory, minedItem, 1)) {
            miner.isMining = false;
            continue;
        }

        if (miner.requiresFuel && miner.fuelRemaining <= 0.0f) {
            if (!tryConsumeFuel(miner, *inventory)) {
                miner.isMining = false;
                continue;
            }
        }

        miner.isMining = true;
        miner.miningProgress += deltaTime * miner.miningSpeed *
                                TileMetadataDatabase::orePatchQualityToSpeedMultiplier(target.quality);

        if (miner.requiresFuel) {
            miner.fuelRemaining -= deltaTime;
            if (miner.fuelRemaining < 0.0f) {
                miner.fuelRemaining = 0.0f;
            }
        }

        while (miner.miningProgress >= 1.0f) {
            if (!canOutputFit(*inventory, minedItem, 1)) {
                miner.isMining = false;
                miner.miningProgress = 1.0f;
                break;
            }

            inventory->outputInventory.addItem(minedItem, 1);
            miner.miningProgress -= 1.0f;
        }
    }
}

MiningSystem::MiningTarget MiningSystem::resolveMiningTarget(const MinerComponent& miner, const PositionComponent& position, const TileMap& tileMap, const TileMetadataDatabase& tileMetadataDatabase, const ItemDatabase& itemDatabase) const {
    const int baseTileX = static_cast<int>(position.position.x) / 32;
    const int baseTileY = static_cast<int>(position.position.y) / 32;

    for (int localY = 0; localY < miner.heightTiles; localY++) {
        for (int localX = 0; localX < miner.widthTiles; localX++) {
            const int tileX = baseTileX + localX;
            const int tileY = baseTileY + localY;

            for (size_t layerIndex = 0; layerIndex < tileMap.getLayers().size(); layerIndex++) {
                const Tile* tile = tileMap.getTile(tileX, tileY, static_cast<int>(layerIndex));
                if (!tile || !tile->shouldRender || tile->paletteName.empty()) {
                    continue;
                }

                const std::string* minedItemName =
                    tileMetadataDatabase.getMinedItemName(tile->paletteName, tile->atlasX, tile->atlasY);
                if (!minedItemName) {
                    continue;
                }

                const ItemDefinition* item = itemDatabase.getItem(*minedItemName);
                if (item) {
                    return {
                        item,
                        tileMetadataDatabase.getOrePatchQuality(tile->paletteName, tile->atlasX, tile->atlasY)
                    };
                }
            }
        }
    }

    return {};
}

bool MiningSystem::canOutputFit(MachineInventoryComponent& inventory, const ItemDefinition* outputItem, int amount) const {
    InventoryGrid& output = inventory.outputInventory;
    if (output.getWidth() <= 0 || output.getHeight() <= 0) {
        return false;
    }

    int remaining = amount;
    for (const auto& slot : output.getSlots()) {
        if (slot.isEmpty()) {
            remaining -= outputItem->maxStackSize;
        } else if (slot.stack.item == outputItem) {
            remaining -= outputItem->maxStackSize - slot.stack.amount;
        }

        if (remaining <= 0) {
            return true;
        }
    }

    return false;
}

bool MiningSystem::tryConsumeFuel(MinerComponent& miner, MachineInventoryComponent& inventory) {
    for (auto& slot : inventory.fuelInventory.getSlots()) {
        if (slot.isEmpty() || !slot.stack.item || slot.stack.item->fuelValue <= 0.0f) {
            continue;
        }

        const ItemDefinition* fuelItem = slot.stack.item;
        if (!inventory.fuelInventory.removeItem(fuelItem, 1)) {
            continue;
        }

        miner.currentFuelCapacity = fuelItem->fuelValue;
        miner.fuelRemaining = fuelItem->fuelValue;
        return true;
    }

    return false;
}
