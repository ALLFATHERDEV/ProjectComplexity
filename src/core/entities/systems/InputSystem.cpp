#include "InputSystem.hpp"
#include <SDL3/SDL_keycode.h>


void InputSystem::handleInput(SDL_Event &event, ComponentStorage<InputComponent> &inputs, ComponentStorage<VelocityComponent> &velocities) {
    if (event.type != SDL_EVENT_KEY_DOWN && event.type != SDL_EVENT_KEY_UP)
        return;

    bool isDown = event.type == SDL_EVENT_KEY_DOWN;

    auto& inputArray = inputs.getRaw();
    auto& entities = inputs.getEntities();

    for (size_t i = 0; i < inputArray.size(); i++) {
        Entity e = entities[i];
        auto& input = inputArray[i];

        auto* vel = velocities.get(e);
        if (!vel) continue;

        switch (event.key.key) {
            case SDLK_W: input.up = isDown; break;
            case SDLK_S: input.down = isDown; break;
            case SDLK_A: input.left = isDown; break;
            case SDLK_D: input.right = isDown; break;
        }
    }
}
