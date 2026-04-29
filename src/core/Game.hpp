#ifndef PROJECTCOMPLEXITY_GAME_H
#define PROJECTCOMPLEXITY_GAME_H

#include <SDL3/SDL.h>

#include "editor/TileMapEditor.hpp"
#include "graphics/Renderer.hpp"
#include "gui/GUISystem.hpp"
#include "gui/craftingmachine/GUIMachine.hpp"
#include "gui/elements/GUIDragPeview.hpp"
#include "world/World.hpp"


class Game {
public:
    Game();
    ~Game();

    void run();
    void events();
    void render();

    static int WINDOW_WIDTH;
    static int WINDOW_HEIGHT;

private:
    SDL_Window* m_Window = nullptr;
    bool m_Running = true;

    GUISystem m_GUISystem;
    GUIMachine m_MachineGUI;
    GUIDragContext m_GUIDragContext;
    GUIDragPreview* m_GUIDragPreview;

    Renderer m_Renderer;
    World m_World;
    TileMapEditor m_TileMapEditor;

    bool isDraggingPlaceableItem() const;
    bool tryPlaceDraggedItem(const SDL_Event& event);
    void renderDraggedPlaceablePreview();
    void renderDebugOverlay();

};

#endif //PROJECTCOMPLEXITY_GAME_H
