#include "MovementSystem.hpp"

void MovementSystem::update(float deltaTime, ComponentStorage<PositionComponent> &positions, ComponentStorage<VelocityComponent> &velocities) {
    auto& velArray = velocities.getRaw();
    auto& entities = velocities.getEntities();

    for (size_t i = 0; i < velArray.size(); i++) {
        Entity e = entities[i];

        auto* pos = positions.get(e);
        if (!pos) continue;

        pos->position.x += velArray[i].velocity.x * deltaTime;
        pos->position.y += velArray[i].velocity.y * deltaTime;
    }
}
