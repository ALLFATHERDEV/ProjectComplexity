#pragma once
#include "../ComponentStorage.hpp"
#include "../../graphics/Renderer.hpp"
#include "../../world/ChunkManager.hpp"
#include "../component/AnimationControllerComponent.hpp"
#include "../component/PositionComponent.hpp"

class Camera2D;

class AnimatedRenderSystem {
public:
    void render(Renderer* renderer, const Camera2D& camera, const ChunkManager& chunkManager, ComponentStorage<PositionComponent>& positions, ComponentStorage<AnimationControllerComponent>& controllers);
};
