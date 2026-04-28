#include "CharacterStateSystem.hpp"

void CharacterStateSystem::update(ComponentStorage<CharacterStateComponent>& states, ComponentStorage<VelocityComponent>& velocities) {
    auto& stateArray = states.getRaw();
    auto& entities = states.getEntities();

    for (size_t i = 0; i < stateArray.size(); i++) {
        Entity entity = entities[i];

        auto* velocity = velocities.get(entity);
        if (!velocity)
            continue;

        auto& state = stateArray[i];

        const float vx = velocity->velocity.x;
        const float vy = velocity->velocity.y;

        if (vx == 0.0f && vy == 0.0f) {
            state.state = CharacterState::IDLE;
            continue;
        }

        state.state = CharacterState::WALK;

        if (std::abs(vx) > std::abs(vy)) {
            state.direction = vx < 0.0f ? Direction::LEFT : Direction::RIGHT;
        } else {
            state.direction = vy < 0.0f ? Direction::UP : Direction::DOWN;
        }
    }
}