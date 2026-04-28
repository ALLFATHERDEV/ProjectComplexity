#include "AnimationLoader.hpp"

SpriteAtlas AnimationLoader::m_PlayerAtlas;

void AnimationLoader::createSpriteAtlas(Renderer* renderer) {
    m_PlayerAtlas.createAtlas(renderer, 32, 32, "assets/Sprite-0002.png");
}

void AnimationLoader::loadPlayerAnimations(AnimationLibrary &library) {
    library.add("player_idle_down", AnimatedSprite({
        m_PlayerAtlas.getSprite(6, 0)
    }, true, 0.2f));
    library.add("player_idle_up", AnimatedSprite({
        m_PlayerAtlas.getSprite(0, 0)
    }, true, 0.2f));
    library.add("player_idle_right", AnimatedSprite({
        m_PlayerAtlas.getSprite(3, 0)
    }, true, 0.2f));
    library.add("player_idle_left", AnimatedSprite({
        m_PlayerAtlas.getSprite(9, 0)
    }, true, 0.2f));

    library.add("player_walk_down", AnimatedSprite({
        m_PlayerAtlas.getSprite(6 ,0),
        m_PlayerAtlas.getSprite(7, 0),
        m_PlayerAtlas.getSprite(8, 0),
        m_PlayerAtlas.getSprite(7, 0),
        m_PlayerAtlas.getSprite(6, 0)
    }, true, 0.2f));

    library.add("player_walk_up", AnimatedSprite({
        m_PlayerAtlas.getSprite(0, 0),
        m_PlayerAtlas.getSprite(1, 0),
        m_PlayerAtlas.getSprite(2, 0),
        m_PlayerAtlas.getSprite(1, 0),
        m_PlayerAtlas.getSprite(0, 0)
    }, true, 0.2f));

    library.add("player_walk_left", AnimatedSprite({
        m_PlayerAtlas.getSprite(9, 0),
        m_PlayerAtlas.getSprite(10, 0),
        m_PlayerAtlas.getSprite(11, 0),
        m_PlayerAtlas.getSprite(10, 0),
        m_PlayerAtlas.getSprite(9, 0)
    }, true, 0.2f));

    library.add("player_walk_right", AnimatedSprite({
        m_PlayerAtlas.getSprite(3, 0),
        m_PlayerAtlas.getSprite(4, 0),
        m_PlayerAtlas.getSprite(5, 0),
        m_PlayerAtlas.getSprite(4, 0),
        m_PlayerAtlas.getSprite(3, 0)
    }, true, 0.2f));
}
