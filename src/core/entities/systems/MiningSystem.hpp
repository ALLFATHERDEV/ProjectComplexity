#pragma once

#include "../ComponentStorage.hpp"
#include "../../inventory/ItemDatabase.hpp"
#include "../../map/TileMetadataDatabase.hpp"
#include "../../map/TileMap.hpp"
#include "../component/MachineInventoryComponent.hpp"
#include "../component/MinerComponent.hpp"
#include "../component/PositionComponent.hpp"

class MiningSystem {
public:
    void update(float deltaTime,
                ComponentStorage<MinerComponent>& miners,
                ComponentStorage<PositionComponent>& positions,
                ComponentStorage<MachineInventoryComponent>& inventories,
                const TileMap& tileMap,
                const TileMetadataDatabase& tileMetadataDatabase,
                const ItemDatabase& itemDatabase);

private:
    struct MiningTarget {
        const ItemDefinition* item = nullptr;
        OrePatchQuality quality = OrePatchQuality::Normal;
    };

    MiningTarget resolveMiningTarget(const MinerComponent& miner, const PositionComponent& position, const TileMap& tileMap,const TileMetadataDatabase& tileMetadataDatabase, const ItemDatabase& itemDatabase) const;
    bool canOutputFit(MachineInventoryComponent& inventory, const ItemDefinition* outputItem, int amount) const;
    bool tryConsumeFuel(MinerComponent& miner, MachineInventoryComponent& inventory);
};
