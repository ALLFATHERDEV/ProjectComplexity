#include "CollisionSystem.hpp"

void CollisionSystem::update(float deltaTime, ComponentStorage<PositionComponent> &positions, ComponentStorage<VelocityComponent> &velocities, ComponentStorage<CollisionComponent> &collisions, const TileMap &tileMap) {
    auto& collisionArray = collisions.getRaw();
    auto& entities = collisions.getEntities();

    for (size_t i = 0; i < collisionArray.size(); i++) {
        Entity entity = entities[i];

        auto* position = positions.get(entity);
        auto* velocity = velocities.get(entity);

        if (!position || !velocity) continue;

        auto& collision = collisionArray[i];
        if (!collision.isBlocking)
            continue;

        moveAxisX(deltaTime, *position, *velocity, collision, tileMap);
        moveAxisY(deltaTime, *position, *velocity, collision, tileMap);
    }
}

SDL_FRect CollisionSystem::getWorldRect(const PositionComponent &position, const CollisionComponent &collision) {
    return {
        position.position.x + collision.bounds.x,
        position.position.y + collision.bounds.y,
        collision.bounds.w,
        collision.bounds.h
    };
}

void CollisionSystem::moveAxisX(float deltaTime, PositionComponent &position, VelocityComponent &velocity, CollisionComponent &collision, const TileMap &tileMap) {
    position.position.x += velocity.velocity.x * deltaTime;

    SDL_FRect rect = getWorldRect(position, collision);
    if (tileMap.isRectColliding(rect)) {
        position.position.x -= velocity.velocity.x * deltaTime;
        velocity.velocity.x = 0.0f;
    }
}

void CollisionSystem::moveAxisY(float deltaTime, PositionComponent &position, VelocityComponent &velocity, CollisionComponent &collision, const TileMap &tileMap) {
    position.position.y += velocity.velocity.y * deltaTime;

    SDL_FRect rect = getWorldRect(position, collision);
    if (tileMap.isRectColliding(rect)) {
        position.position.y -= velocity.velocity.y * deltaTime;
        velocity.velocity.y = 0.0f;
    }
}
