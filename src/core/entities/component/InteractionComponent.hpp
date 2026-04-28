#pragma once

#include <string>
#include <SDL3/SDL.h>

struct InteractionComponent {
    std::string interactionName = "Interact";
    SDL_FRect interactionBounds{0, 0, 32, 32};
};