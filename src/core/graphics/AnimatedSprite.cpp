#include "AnimatedSprite.hpp"

#include <utility>

#include "../Logger.hpp"

AnimatedSprite::AnimatedSprite() {
}

AnimatedSprite::AnimatedSprite(std::vector<Sprite> sprites, const bool loop, float playTime) {
    m_Sprites = std::move(sprites);
    m_Loop = loop;
    m_MaxFrames = m_Sprites.size();
    m_PlayTime = playTime;
}

void AnimatedSprite::play() {
    if (!m_IsPlaying) {
        m_CurrentFrame = 0;
        m_IsPlaying = true;
    } else {
        LOG_WARN("Tried to start playing the animation again, but it was already started");
    }
}

void AnimatedSprite::stop() {
    if (m_IsPlaying) {
        m_IsPlaying = false;
    } else {
        LOG_WARN("Tried to stop the animation, but it was already stopped");
    }
}

void AnimatedSprite::update(float deltaTime) {
    if (m_IsPlaying) {
        if (m_FrameTime >= m_PlayTime) {
            m_FrameTime = 0;
            m_CurrentFrame += 1;
            if (m_CurrentFrame == m_MaxFrames) {
                if (m_Loop) {
                    m_CurrentFrame = 0;
                } else {
                    m_IsPlaying = false;
                }
            }
        }
        m_FrameTime += deltaTime;
    }
}

void AnimatedSprite::reset() {
    m_CurrentFrame = 0;
    m_FrameTime = 0.0f;
}


void AnimatedSprite::render(Renderer* renderer, SDL_FRect destRect) {
    if (m_IsPlaying) {
        renderer->drawSprite(m_Sprites[m_CurrentFrame], destRect);
    }
}

