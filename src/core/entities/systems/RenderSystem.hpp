#pragma once
#include "../ComponentStorage.hpp"
#include "../../camera/Camera2D.hpp"
#include "../component/PositionComponent.hpp"
#include "../component/SpriteComponent.hpp"
#include "../../graphics/Renderer.hpp"

class RenderSystem {
public:
    void render(Renderer* renderer, const Camera2D& camera, ComponentStorage<PositionComponent>& positions, ComponentStorage<SpriteComponent>& sprites);

private:
    int m_CellWidth = 32;
    int m_CellHeight = 32;
};
