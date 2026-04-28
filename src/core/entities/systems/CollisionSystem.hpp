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
    SDL_FRect getWorldRect(const PositionComponent& position, const CollisionComponent& collision);
    void moveAxisX(float deltaTime, PositionComponent& position, VelocityComponent& velocity, CollisionComponent& collision, const TileMap& tileMap);
    void moveAxisY(float deltaTime, PositionComponent& position, VelocityComponent& velocity, CollisionComponent& collision, const TileMap& tileMap);
};
