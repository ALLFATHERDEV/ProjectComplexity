#pragma once

#include <unordered_map>
#include <string>
#include "../graphics/AnimatedSprite.hpp"

class AnimationLibrary {
public:
    void add(const std::string& name, AnimatedSprite animation) {
        m_Animations.emplace(name, animation);
    }

    AnimatedSprite* get(const std::string& name) {
        auto it = m_Animations.find(name);
        if (it == m_Animations.end())
            return nullptr;
        return &it->second;
    }

private:
    std::unordered_map<std::string, AnimatedSprite> m_Animations;
};