#pragma once

#include "CharacterStateComponent.hpp"
#include <string>
#include <unordered_map>

#include "../../graphics/AnimatedSprite.hpp"

struct AnimationKey {
    std::string stateName;
    bool useDirection = false;
    Direction direction = Direction::DOWN;

    bool operator==(const AnimationKey& other) const {
        return stateName == other.stateName &&
               useDirection == other.useDirection &&
               direction == other.direction;
    }
};

struct AnimationKeyHash {
    std::size_t operator()(const AnimationKey& key) const {
        std::size_t hash = std::hash<std::string>{}(key.stateName);
        hash ^= static_cast<std::size_t>(key.useDirection) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= static_cast<std::size_t>(key.direction) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        return hash;
    }
};

struct AnimationControllerComponent {
    std::unordered_map<AnimationKey, AnimatedSprite, AnimationKeyHash> animations;

    std::string stateName = "default";
    bool useDirection = false;
    Direction direction = Direction::DOWN;
    bool enabled = true;
    int sortOrder = 0;
    float renderWidth = 0.0f;
    float renderHeight = 0.0f;
    bool centerInSourceRect = false;
    AnimatedSprite* currentAnimation = nullptr;
};
