#pragma once

#include <SDL3/SDL.h>

struct CollisionComponent {
    SDL_FRect bounds{};

    bool isBlocking = true;
    bool isTrigger = false;
};
