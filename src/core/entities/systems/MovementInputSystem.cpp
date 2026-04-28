#include "MovementInputSystem.hpp"

void MovementInputSystem::update(ComponentStorage<InputComponent> &inputs, ComponentStorage<VelocityComponent> &velocities) {
    auto& inputArray = inputs.getRaw();
    auto& entities = inputs.getEntities();

    for (size_t i = 0; i < inputArray.size(); i++) {
        Entity e = entities[i];
        auto& input = inputArray[i];

        auto* vel = velocities.get(e);
        if (!vel) continue;

        float x = 0.0f;
        float y = 0.0f;

        if (input.left)  x -= 1.0f;
        if (input.right) x += 1.0f;
        if (input.up)    y -= 1.0f;
        if (input.down)  y += 1.0f;

        // 🔥 Optional: Normalisieren (für diagonale Bewegung)
        if (x != 0.0f || y != 0.0f) {
            float length = sqrt(x * x + y * y);
            x /= length;
            y /= length;
        }

        vel->velocity.x = x * input.speed;
        vel->velocity.y = y * input.speed;
    }
}
