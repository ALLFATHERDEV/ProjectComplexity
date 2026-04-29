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

void Camera2D::setZoom(float zoom) {
    if (zoom < 0.25f) {
        zoom = 0.25f;
    }

    if (zoom > 4.0f) {
        zoom = 4.0f;
    }

    m_Zoom = zoom;
}

void Camera2D::addZoom(float delta) {
    setZoom(m_Zoom + delta);
}

float Camera2D::getX() const {
    return m_Position.x;
}

float Camera2D::getY() const {
    return m_Position.y;
}

float Camera2D::getZoom() const {
    return m_Zoom;
}
