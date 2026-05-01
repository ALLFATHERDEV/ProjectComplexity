#pragma once

#include <string>

#include "../GUIElement.hpp"

class GUIText : public GUIElement {
public:
    void setText(const std::string& text) {
        m_Text = text;
    }

    void setColor(SDL_Color color) {
        m_Color = color;
    }

    void render(Renderer* renderer) override {
        if (m_Text.empty()) {
            return;
        }

        renderer->drawText(m_Text, m_Position.x, m_Position.y, m_Color);
    }

private:
    std::string m_Text;
    SDL_Color m_Color{255, 255, 255, 255};
};
