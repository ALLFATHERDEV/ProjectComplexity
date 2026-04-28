#pragma once
#include "../ComponentStorage.hpp"
#include "../../graphics/Renderer.hpp"
#include "../component/AnimationControllerComponent.hpp"
#include "../component/PositionComponent.hpp"

class Camera2D;

class AnimatedRenderSystem {
public:
    void render(Renderer* renderer, const Camera2D& camera, ComponentStorage<PositionComponent>& positions, ComponentStorage<AnimationControllerComponent>& controllers);
};
