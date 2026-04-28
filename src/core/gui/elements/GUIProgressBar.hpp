#pragma once
#include "../GUIElement.hpp"
#include <algorithm>

class GUIProgressBar : public GUIElement {
public:
    void setValue(float value) {
        m_Value = std::clamp(value, 0.0f, 1.0f);
    }

    void setBackgroundColor(SDL_Color color) {
        m_BackgroundColor = color;
    }

    void setFillColor(SDL_Color color) {
        m_FillColor = color;
    }

    void render(Renderer* renderer) override {
        renderer->drawFilledRect(m_Position, m_BackgroundColor);
        renderer->drawRect(m_Position, SDL_Color{140, 140, 140, 255});

        SDL_FRect fillRect = m_Position;
        fillRect.w = m_Position.w * m_Value;

        renderer->drawFilledRect(fillRect, m_FillColor);
    }
private:
    float m_Value = 0.0f;
    SDL_Color m_BackgroundColor{35, 35, 35, 255};
    SDL_Color m_FillColor{80, 180, 80, 255};
};