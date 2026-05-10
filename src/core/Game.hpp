#ifndef PROJECTCOMPLEXITY_GAME_H
#define PROJECTCOMPLEXITY_GAME_H

#include <memory>

#include <SDL3/SDL.h>

#include "graphics/Renderer.hpp"
#include "screen/IScreen.hpp"
#include "screen/ScreenContext.hpp"

class Game {
public:
    Game();
    ~Game();

    void run();
    void events();
    void render();
    void changeScreen(std::unique_ptr<IScreen> screen);
    void requestQuit() { m_Running = false; }

    static int WINDOW_WIDTH;
    static int WINDOW_HEIGHT;

private:
    SDL_Window* m_Window = nullptr;
    bool m_Running = true;

    Renderer m_Renderer;
    ScreenContext m_ScreenContext;
    std::unique_ptr<IScreen> m_CurrentScreen;

};

#endif //PROJECTCOMPLEXITY_GAME_H
