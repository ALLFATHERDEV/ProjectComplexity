#pragma once
#include "../ComponentStorage.hpp"
#include "../../graphics/Renderer.hpp"
#include "../../world/ChunkManager.hpp"
#include "../component/AnimationControllerComponent.hpp"
#include "../component/PositionComponent.hpp"
#include <vector>

class Camera2D;

class AnimatedRenderSystem {
public:
    static void render(Renderer* renderer, const Camera2D& camera, const ChunkManager& chunkManager, ComponentStorage<PositionComponent>& positions, ComponentStorage<AnimationControllerComponent>& controllers);

private:

    static std::vector<size_t> m_DrawOder;
    static size_t m_LastSpriteCount;
    static bool m_DrawOrderDirty;
};
