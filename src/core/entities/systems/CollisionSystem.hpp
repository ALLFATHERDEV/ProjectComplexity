#pragma once
#include "../ComponentStorage.hpp"
#include "../../map/TileMap.hpp"
#include "../component/CollisionComponent.hpp"
#include "../component/PositionComponent.hpp"
#include "../component/VelocityComponent.hpp"

class CollisionSystem {
public:
    void update(float deltaTime, ComponentStorage<PositionComponent>& positions, ComponentStorage<VelocityComponent>& velocities, ComponentStorage<CollisionComponent>& collisions, const TileMap& tileMap);

private:
    SDL_FRect getWorldRect(const PositionComponent& position, const CollisionComponent& collision) const;
    bool collidesWithBlockingEntity(Entity self, const SDL_FRect& rect, ComponentStorage<PositionComponent>& positions, ComponentStorage<CollisionComponent>& collisions) const;
    void moveAxisX(float deltaTime, Entity entity, PositionComponent& position, VelocityComponent& velocity, CollisionComponent& collision, ComponentStorage<PositionComponent>& positions, ComponentStorage<CollisionComponent>& collisions, const TileMap& tileMap);
    void moveAxisY(float deltaTime, Entity entity, PositionComponent& position, VelocityComponent& velocity, CollisionComponent& collision, ComponentStorage<PositionComponent>& positions, ComponentStorage<CollisionComponent>& collisions, const TileMap& tileMap);
};
