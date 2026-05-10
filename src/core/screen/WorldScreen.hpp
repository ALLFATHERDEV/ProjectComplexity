#pragma once
#include "IScreen.hpp"
#include "ScreenContext.hpp"
#include "../editor/ItemDebugEditor.hpp"
#include "../editor/TileMapEditor.hpp"
#include "../gui/GUISystem.hpp"
#include "../gui/craftingmachine/GUIMachine.hpp"
#include "../gui/elements/GUIDragPeview.hpp"
#include "../util/MathUtil.hpp"
#include "../world/World.hpp"

class WorldScreen : public IScreen {
public:
    explicit WorldScreen(ScreenContext& context);

    void onEnter() override;
    void onExit() override;

    void update(float deltaTime) override;
    void render() override;
    void handleEvent(SDL_Event& event) override;

private:
    bool isDraggingPlaceableItem() const;
    bool isDraggingConveyorPlaceableItem() const;
    bool tryPlaceDraggedItem(const SDL_Event& event);
    void renderDraggedPlaceablePreview();
    void renderDebugOverlay();
    Vec2f getMouseWorldPosition();
    void rotateSelectedPlaceableDirection();

    ScreenContext& m_Context;

    GUISystem m_GUISystem;
    GUIMachine m_MachineGUI;
    GUIDragContext m_GUIDragContext;
    GUIDragPreview* m_GUIDragPreview = nullptr;

    World m_World;
    TileMapEditor m_TileMapEditor;
    ItemDebugEditor m_ItemDebugEditor;
    Direction m_SelectedPlaceableDirection = Direction::RIGHT;

    float m_DebugFps = 0.0f;
    float m_DebugFpsAccumulator = 0.0f;
    int m_DebugFpsFrameCount = 0;

};
