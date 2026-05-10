#pragma once

#include <SDL3/SDL.h>

class IScreen {
public:
    virtual ~IScreen() = default;

    virtual void onEnter() {};
    virtual void onExit() {};

    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleEvent(SDL_Event& event) = 0;
};