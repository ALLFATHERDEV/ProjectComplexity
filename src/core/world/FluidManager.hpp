#pragma once

#include "../camera/Camera2D.hpp"
#include "../entities/ComponentRegistry.hpp"
#include "../entities/EntityManager.hpp"
#include "../entities/component/FluidPipeComponent.hpp"
#include "../entities/component/CollisionComponent.hpp"
#include "../entities/systems/FluidSystem.hpp"
#include "../fluid/FluidDefinition.hpp"
#include "../graphics/Renderer.hpp"
#include "AnimationLibrary.hpp"
#include "../graphics/SpriteAtlas.hpp"

#include <unordered_map>

class ChunkManager;

struct AtlasCoords {
    int x;
    int y;
};

struct PipeSpriteLayout {
    AtlasCoords isolatedVertical{0, 0};
    AtlasCoords isolatedHorizontal{1, 0};
    AtlasCoords endUp{8, 0};
    AtlasCoords endRight{9, 0};
    AtlasCoords endDown{6, 0};
    AtlasCoords endLeft{7, 0};
    AtlasCoords vertical{0, 0};
    AtlasCoords horizontal{1, 0};
    AtlasCoords cornerUpRight{3, 0};
    AtlasCoords cornerRightDown{4, 0};
    AtlasCoords cornerDownLeft{5, 0};
    AtlasCoords cornerLeftUp{2, 0};
    AtlasCoords teeNoLeft{12, 0};
    AtlasCoords teeNoUp{13, 0};
    AtlasCoords teeNoRight{14, 0};
    AtlasCoords teeNoDown{11, 0};
    AtlasCoords cross{10, 0};
    AtlasCoords tank{6, 1};
    AtlasCoords pumpUp{6, 3};
    AtlasCoords pumpRight{7, 3};
    AtlasCoords pumpDown{8, 3};
    AtlasCoords pumpLeft{9, 3};
};

class FluidManager {
public:
    FluidManager(EntityManager& entityManager,
                 ComponentRegistry& components,
                 AnimationLibrary& animationLibrary,
                 FluidSystem& fluidSystem);

    void placeFluidPipe(int tileX, int tileY, Direction direction);
    void removeFluidPipe(int tileX, int tileY);
    void placeFluidTank(int tileX, int tileY);
    void removeFluidTank(int tileX, int tileY);
    void placeFluidPump(int tileX, int tileY, Direction direction);
    void removeFluidPump(int tileX, int tileY);
    bool addDebugFluidToTank(int tileX, int tileY, float amount);
    void renderDebug(Renderer& renderer, const Camera2D& camera, const ChunkManager& chunkManager) const;
    void setAtlas(SpriteAtlas* atlas);
    void registerFluidTankEntity(int tileX, int tileY, Entity entity);
    void registerFluidPumpEntity(int tileX, int tileY, Entity entity);
    void unregisterFluidTankEntity(int tileX, int tileY);
    void unregisterFluidPumpEntity(int tileX, int tileY);
    void setDefaultFluidDefinition(const FluidDefinition* fluidDefinition);
    const FluidDefinition* getDebugFluidDefinition() const;

private:
    static long long makeTileKey(int tileX, int tileY);
    Entity getFluidPipeAtTile(int tileX, int tileY) const;
    Entity getFluidTankAtTile(int tileX, int tileY) const;
    Entity getFluidPumpAtTile(int tileX, int tileY) const;
    void refreshFluidSpriteAt(int tileX, int tileY);
    void refreshFluidSpritesAround(int tileX, int tileY);
    bool hasConnectableNeighbor(int tileX, int tileY, Direction direction) const;
    bool hasCompatiblePortAtTile(int tileX, int tileY, Direction direction) const;
    Sprite getPipeSprite(const FluidPipeComponent& pipe) const;
    Sprite getTankSprite() const;
    Sprite getPumpSprite(Direction direction) const;

    EntityManager& m_EntityManager;
    ComponentRegistry& m_Components;
    AnimationLibrary& m_AnimationLibrary;
    FluidSystem& m_FluidSystem;
    SpriteAtlas* m_FluidAtlas = nullptr;
    std::unordered_map<long long, Entity> m_FluidPipeEntitiesByTile;
    std::unordered_map<long long, Entity> m_FluidTankEntitiesByTile;
    std::unordered_map<long long, Entity> m_FluidPumpEntitiesByTile;
    const FluidDefinition* m_DefaultFluidDefinition = nullptr;
    FluidDefinition m_DebugWater{"water", "Water"};

    float kTileSize = 32.0f;
};
