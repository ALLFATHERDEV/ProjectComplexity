#pragma once

#include "CharacterStateComponent.hpp"
#include <unordered_map>

#include "../../graphics/AnimatedSprite.hpp"

struct AnimationKey {
    CharacterState state;
    Direction direction;

    bool operator==(const AnimationKey& other) const {
        return state == other.state && direction == other.direction;
    }
};

struct AnimationKeyHash {
    std::size_t operator()(const AnimationKey& key) const {
        return (static_cast<int>(key.state) << 4) ^ static_cast<int>(key.direction);
    }
};

struct AnimationControllerComponent {
    std::unordered_map<AnimationKey, AnimatedSprite*, AnimationKeyHash> animations;

    AnimatedSprite* currentAnimation = nullptr;
};
