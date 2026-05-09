#pragma once

#include "AnimationLibrary.hpp"
#include "../entities/ComponentRegistry.hpp"
#include "../entities/EntityManager.hpp"
#include "../entities/component/CollisionComponent.hpp"
#include "../graphics/SpriteAtlas.hpp"

#include <tuple>
#include <unordered_map>
#include <vector>

struct AnimationControllerComponent;

struct SpriteCoords {
    int x;
    int y;
};

class ConveyorManager {
public:
    ConveyorManager(EntityManager& entityManager,
                    ComponentRegistry& components,
                    AnimationLibrary& animationLibrary);

    void setAtlas(SpriteAtlas* atlas);
    void placeConveyorBelt(int tileX, int tileY, Direction direction);
    void removeConveyorBelt(int tileX, int tileY);
    void clearConveyorBelts();
    std::vector<std::tuple<int, int, Direction>> getConveyorBeltData() const;

private:
    static long long makeTileKey(int tileX, int tileY);
    Entity getConveyorBeltAt(int tileX, int tileY) const;
    void refreshConveyorSpriteAt(int tileX, int tileY);
    void refreshConveyorSpritesAround(int tileX, int tileY);
    bool pointsTo(Direction direction, int fromX, int fromY, int targetX, int targetY);
    SpriteCoords getStraightConveyorSprite(Direction direction);
    SpriteCoords getCurveConveyorSprite(Direction incomingSide, Direction outgoingDirection);

    EntityManager& m_EntityManager;
    ComponentRegistry& m_Components;
    AnimationLibrary& m_AnimationLibrary;
    SpriteAtlas* m_ConveyorAtlas = nullptr;
    std::unordered_map<long long, Entity> m_ConveyorEntitiesByTile;

    float kTileSize = 32.0f;
};
