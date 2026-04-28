#include "InteractionSystem.hpp"

#include "../../Logger.hpp"

std::optional<Entity> InteractionSystem::handleInput(const SDL_Event &event, Entity player, ComponentStorage<PositionComponent> &positions, ComponentStorage<InteractionComponent> &interactions) {
    if (event.type != SDL_EVENT_KEY_DOWN)
        return std::nullopt;
    if (event.key.key != SDLK_E)
        return std::nullopt;

    auto* playerPos = positions.get(player);
    if (!playerPos)
        return std::nullopt;

    SDL_FRect playerRect{static_cast<float>(playerPos->position.x), static_cast<float>(playerPos->position.y), 32.0f, 32.0f};

    auto& interactionArray = interactions.getRaw();
    auto& entities = interactions.getEntities();

    for (size_t i = 0; i < interactionArray.size(); i++) {
        Entity entity = entities[i];

        auto* entityPosition = positions.get(entity);
        if (!entityPosition)
            continue;

        auto& interaction = interactionArray[i];

        SDL_FRect interactionRect{
            entityPosition->position.x + interaction.interactionBounds.x,
            entityPosition->position.y + interaction.interactionBounds.y,
            interaction.interactionBounds.w,
            interaction.interactionBounds.h
        };

        if (intersects(playerRect, interactionRect)) {
            LOG_INFO("Interacted with {}", interaction.interactionName);
            return entity;
        }
    }
    return std::nullopt;
}

bool InteractionSystem::intersects(const SDL_FRect &a, const SDL_FRect &b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}
