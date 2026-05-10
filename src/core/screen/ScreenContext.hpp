#pragma once

#include <SDL3/SDL.h>

class Renderer;
class Game;

struct ScreenContext {
    Game& game;
    Renderer& renderer;
    SDL_Window* window = nullptr;
};
