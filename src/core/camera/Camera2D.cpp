#include "Camera2D.hpp"

void Camera2D::setPosition(const Vec2f &position) {
    m_Position = position;
}

Vec2f Camera2D::getPosition() const {
    return m_Position;
}

void Camera2D::move(const Vec2f &offset) {
    m_Position.x += offset.x;
    m_Position.y += offset.y;
}

float Camera2D::getX() const {
    return m_Position.x;
}

float Camera2D::getY() const {
    return m_Position.y;
}
