#pragma once
#include "../util/MathUtil.hpp"

class Camera2D {
public:

    void setPosition(const Vec2f& position);
    Vec2f getPosition() const;
    void move(const Vec2f& offset);
    void setZoom(float zoom);
    void addZoom(float delta);
    float getX() const;
    float getY() const;
    float getZoom() const;

private:
    Vec2f m_Position{0.0f, 0.0f};
    float m_Zoom = 1.0f;
};
