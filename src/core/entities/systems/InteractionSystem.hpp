#pragma once

#include <optional>
#include "../ComponentStorage.hpp"
#include "../component/InteractionComponent.hpp"
#include "../component/PositionComponent.hpp"
#include "SDL3/SDL_events.h"

class InteractionSystem {
public:
    std::optional<Entity> handleInput(const SDL_Event& event, Entity player, ComponentStorage<PositionComponent>& positions, ComponentStorage<InteractionComponent>& interactions);

private:
    bool intersects(const SDL_FRect& a, const SDL_FRect& b);
};
