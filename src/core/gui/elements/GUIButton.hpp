#pragma once
#include <functional>

#include "../GUIElement.hpp"

class GUIButton : public GUIElement {
public:
    void setText(const std::string& text) {
        m_Text = text;
    }

    void setOnClick(std::function<void()> onClick) {
        m_OnClick = std::move(onClick);
    }

    void setColors(SDL_Color normalColor, SDL_Color hoverColor) {
        m_NormalColor = normalColor;
        m_HoverColor = hoverColor;
    }

    void handleEvent(const SDL_Event &event) override {
        if (!isVisible()) {
            return;
        }

        if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN)
            return;

        if (event.button.button != SDL_BUTTON_LEFT)
            return;

        float mouseX = event.button.x;
        float mouseY = event.button.y;

        if (containsPoint(mouseX, mouseY))
            if (m_OnClick)
                m_OnClick();
    }

    void render(Renderer *renderer) override {
        if (!isVisible()) {
            return;
        }

        float mouseX;
        float mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        bool hovered = containsPoint(mouseX, mouseY);

        renderer->drawFilledRect(m_Position, hovered ? m_HoverColor : m_NormalColor);
        renderer->drawRect(m_Position, SDL_Color{140, 140, 140, 255});
        renderer->drawText(m_Text, m_Position.x + 8.0f, m_Position.y + 7.0f, SDL_Color{255, 255, 255, 255});

    }

private:
    std::string m_Text;
    std::function<void()> m_OnClick;

    SDL_Color m_NormalColor{50, 50, 50, 255};
    SDL_Color m_HoverColor{100, 100, 100, 255};
};
