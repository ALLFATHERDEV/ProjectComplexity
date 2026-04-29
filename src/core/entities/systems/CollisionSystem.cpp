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

        moveAxisX(deltaTime, entity, *position, *velocity, collision, positions, collisions, tileMap);
        moveAxisY(deltaTime, entity, *position, *velocity, collision, positions, collisions, tileMap);
    }
}

SDL_FRect CollisionSystem::getWorldRect(const PositionComponent &position, const CollisionComponent &collision) const {
    return {
        position.position.x + collision.bounds.x,
        position.position.y + collision.bounds.y,
        collision.bounds.w,
        collision.bounds.h
    };
}

bool CollisionSystem::collidesWithBlockingEntity(Entity self, const SDL_FRect& rect, ComponentStorage<PositionComponent>& positions, ComponentStorage<CollisionComponent>& collisions) const {
    const auto& collisionArray = collisions.getRaw();
    const auto& entities = collisions.getEntities();

    for (size_t i = 0; i < collisionArray.size(); i++) {
        const Entity other = entities[i];
        if (other == self) {
            continue;
        }

        const CollisionComponent& otherCollision = collisionArray[i];
        if (!otherCollision.isBlocking || otherCollision.isTrigger) {
            continue;
        }

        const PositionComponent* otherPosition = positions.get(other);
        if (!otherPosition) {
            continue;
        }

        const SDL_FRect otherRect = getWorldRect(*otherPosition, otherCollision);
        if (SDL_HasRectIntersectionFloat(&rect, &otherRect)) {
            return true;
        }
    }

    return false;
}

void CollisionSystem::moveAxisX(float deltaTime, Entity entity, PositionComponent &position, VelocityComponent &velocity, CollisionComponent &collision, ComponentStorage<PositionComponent>& positions, ComponentStorage<CollisionComponent>& collisions, const TileMap &tileMap) {
    position.position.x += velocity.velocity.x * deltaTime;

    const SDL_FRect rect = getWorldRect(position, collision);
    if (tileMap.isRectColliding(rect) || collidesWithBlockingEntity(entity, rect, positions, collisions)) {
        position.position.x -= velocity.velocity.x * deltaTime;
        velocity.velocity.x = 0.0f;
    }
}

void CollisionSystem::moveAxisY(float deltaTime, Entity entity, PositionComponent &position, VelocityComponent &velocity, CollisionComponent &collision, ComponentStorage<PositionComponent>& positions, ComponentStorage<CollisionComponent>& collisions, const TileMap &tileMap) {
    position.position.y += velocity.velocity.y * deltaTime;

    const SDL_FRect rect = getWorldRect(position, collision);
    if (tileMap.isRectColliding(rect) || collidesWithBlockingEntity(entity, rect, positions, collisions)) {
        position.position.y -= velocity.velocity.y * deltaTime;
        velocity.velocity.y = 0.0f;
    }
}
