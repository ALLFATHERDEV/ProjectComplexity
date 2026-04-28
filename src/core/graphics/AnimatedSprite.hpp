#ifndef PROJECTCOMPLEXITY_ANIMATEDSPRITE_H
#define PROJECTCOMPLEXITY_ANIMATEDSPRITE_H

#include <vector>

#include "Renderer.hpp"
#include "Sprite.hpp"

class AnimatedSprite {
public:
    AnimatedSprite();
    AnimatedSprite(std::vector<Sprite> sprites, bool loop, float playTime);

    void play();
    void stop();
    void render(Renderer* renderer, SDL_FRect destRect);
    void update(float deltaTime);
    void reset();
    bool isPlaying() { return m_IsPlaying; }

private:
    std::vector<Sprite> m_Sprites;
    bool m_Loop;
    bool m_IsPlaying = false;
    int m_CurrentFrame = 0;
    int m_MaxFrames;
    float m_FrameTime = 0.0f;
    float m_PlayTime = 0.0f;

};

#endif //PROJECTCOMPLEXITY_ANIMATEDSPRITE_H