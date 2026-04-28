#pragma once
#include "../graphics/Renderer.hpp"

class GUIElement {
public:
    virtual ~GUIElement() = default;

    virtual void update(float deltaTime) {}
    virtual void render(Renderer* renderer) {}
    virtual void handleEvent(const SDL_Event& event) {}

    void setPosition(float x, float y) {
        m_Position.x = x;
        m_Position.y = y;
    }

    void setSize(float w, float h) {
        m_Position.w = w;
        m_Position.h = h;
    }

    bool containsPoint(float x, float y) const {
        return x >= m_Position.x && y >= m_Position.y && x <= m_Position.x + m_Position.w && y <= m_Position.y + m_Position.h;
    }

    void setVisible(bool visible) { m_Visible = visible; }
    bool isVisible() const { return m_Visible; }

protected:
    SDL_FRect m_Position{0, 0, 0, 0};
    bool m_Visible = true;
};
