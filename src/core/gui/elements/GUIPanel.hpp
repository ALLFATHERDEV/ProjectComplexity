#pragma once
#include "../GUIElement.hpp"

class GUIPanel : public GUIElement {
public:
    void setColor(SDL_Color color) {
        m_Color = color;
    }

    void render(Renderer *renderer) override {
        renderer->drawFilledRect(m_Position, m_Color);
    }

private:
    SDL_Color m_Color{30, 30, 30, 220};
};