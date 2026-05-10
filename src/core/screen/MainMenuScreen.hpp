#pragma once

#include "IScreen.hpp"
#include "ScreenContext.hpp"

class MainMenuScreen : public IScreen {
public:
    explicit MainMenuScreen(ScreenContext& context);

    void update(float deltaTime) override;
    void render() override;
    void handleEvent(SDL_Event& event) override;

private:
    ScreenContext& m_Context;
};
