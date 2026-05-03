#include "Game.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "Logger.hpp"

int Game::WINDOW_WIDTH = 1800;
int Game::WINDOW_HEIGHT = 1200;

Game::Game(){
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_ERROR("Failed to initialize SDL: {}", SDL_GetError());
        exit(1);
    }

    m_Window = SDL_CreateWindow("CozyGame", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

    if (!m_Window) {
        LOG_ERROR("Failed to create window: {}", SDL_GetError());
        exit(1);
    }

    m_Renderer.init(m_Window);
    m_Renderer.loadFont("assets/font/orbitron.ttf", 18.0f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(m_Window, m_Renderer.getSDLRenderer());
    ImGui_ImplSDLRenderer3_Init(m_Renderer.getSDLRenderer());

    m_World.setRenderer(&m_Renderer);
    m_World.initializeWorld();
    m_TileMapEditor.clearTilePalettes();
    for (const auto& palette : m_World.getTilePalettes()) {
        m_TileMapEditor.addTilePalette(palette.name, palette.atlas);
    }

    m_MachineGUI.create(m_GUISystem, &m_GUIDragContext);
    m_MachineGUI.bind(&m_World.getMachineInventories(),
                      &m_World.getCraftingMachines(),
                      &m_World.getMiners(),
                      &m_World.getRecipeDatabase(),
                      &m_World.getItemDatabase(),
                      &m_World.getInventories(),
                      m_World.getPlayer());

    m_GUIDragPreview = m_GUISystem.addElement<GUIDragPreview>();
    m_GUIDragPreview->setDragContext(&m_GUIDragContext);


    LOG_INFO("Game initialized");
}

Game::~Game() {

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    m_Renderer.clearRenderer();
    SDL_DestroyWindow(m_Window);
    SDL_Quit();
}

void Game::run() {

    Uint64 previousTicks = SDL_GetTicks();

    while (m_Running) {
        //Calculating deltaTime
        Uint64 currentTicks = SDL_GetTicks();
        Uint64 deltaTimeMs = currentTicks - previousTicks;
        previousTicks = currentTicks;
        float deltaTime = static_cast<float>(deltaTimeMs) / 1000.0f;

        events();
        m_World.update(deltaTime);
        m_MachineGUI.update();
        render();
    }

}

void Game::events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_QUIT) {
           m_Running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F1) {
            m_TileMapEditor.setEnabled(!m_TileMapEditor.isEnabled());
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F2) {
            m_ItemDebugEditor.setEnabled(!m_ItemDebugEditor.isEnabled());
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            m_World.getCamera().addZoom(event.wheel.y * 0.1f);
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_R && isDraggingConveyorPlaceableItem()) {
            rotateSelectedPlaceableDirection();
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
            m_GUIDragContext.suppressWorldPlacementUntilMouseRelease = false;
        }

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse && !isDraggingPlaceableItem()) {
            m_TileMapEditor.update(event, m_World, m_World.getCamera());
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB) {
            m_MachineGUI.togglePlayerInventory();
        }
        m_GUISystem.handleEvent(event);

        if (!io.WantCaptureMouse) {
            tryPlaceDraggedItem(event);
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            if (m_MachineGUI.isOpen() || m_MachineGUI.isPlayerInventoryOpen()) {
                m_MachineGUI.close();
            }
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_E) {
            auto interactedEntity = m_World.tryInteract(event);
            if (interactedEntity.has_value()) {
                if (m_World.getMachineInventories().get(interactedEntity.value())) {
                    m_MachineGUI.open(interactedEntity.value());
                } else if (m_World.getInventories().get(interactedEntity.value()) &&
                           interactedEntity.value() != m_World.getPlayer()) {
                    m_MachineGUI.openStorage(interactedEntity.value());
                }
            }
        }

        if (!io.WantCaptureKeyboard) {
            m_World.handleInput(event);
        }

    }
}

void Game::render() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    SDL_RenderClear(m_Renderer.getSDLRenderer());
    m_World.render();

    Vec2f worldPos = getMouseWorldPosition();
    if (const Entity hovered = m_World.getHoveredMachine(worldPos.x, worldPos.y); hovered > 0) {
        m_World.renderMachineHighlight(hovered);
    }

    renderDraggedPlaceablePreview();
    m_GUISystem.render(&m_Renderer);

    m_TileMapEditor.renderImGui(m_World);
    m_ItemDebugEditor.renderImGui(m_World);
    renderDebugOverlay();

    ImGui::Render();

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer.getSDLRenderer());

    SDL_RenderPresent(m_Renderer.getSDLRenderer());
}

bool Game::isDraggingPlaceableItem() const {
    if (!m_GUIDragContext.isDragging || m_GUIDragContext.draggedStack.isEmpty() || !m_GUIDragContext.draggedStack.item) {
        return false;
    }

    return m_GUIDragContext.draggedStack.item->isPlaceable;
}

bool Game::isDraggingConveyorPlaceableItem() const {
    if (!isDraggingPlaceableItem()) {
        return false;
    }

    return m_GUIDragContext.draggedStack.item->placesConveyorBelt;
}

bool Game::tryPlaceDraggedItem(const SDL_Event& event) {
    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT) {
        return false;
    }

    if (!isDraggingPlaceableItem()) {
        return false;
    }

    if (m_GUIDragContext.suppressWorldPlacementUntilMouseRelease) {
        return false;
    }

    const ItemDefinition* item = m_GUIDragContext.draggedStack.item;
    if (!item) {
        return false;
    }

    float mouseX = 0.0f;
    float mouseY = 0.0f;
    SDL_GetMouseState(&mouseX, &mouseY);

    Vec2f worldPosition = getMouseWorldPosition();
    const int tileX = static_cast<int>(worldPosition.x) / 32;
    const int tileY = static_cast<int>(worldPosition.y) / 32;

    if (!m_World.placeItem(*item, tileX, tileY, m_SelectedPlaceableDirection)) {
        return false;
    }

    m_GUIDragContext.draggedStack.amount--;
    if (m_GUIDragContext.draggedStack.amount <= 0) {
        m_GUIDragContext.clear();
    }

    return true;
}

void Game::renderDraggedPlaceablePreview() {
    if (!isDraggingPlaceableItem()) {
        return;
    }

    const ItemDefinition* item = m_GUIDragContext.draggedStack.item;
    if (!item) {
        return;
    }

    float mouseX = 0.0f;
    float mouseY = 0.0f;
    SDL_GetMouseState(&mouseX, &mouseY);

    Vec2f worldPosition = getMouseWorldPosition();
    const int tileX = static_cast<int>(worldPosition.x) / 32;
    const int tileY = static_cast<int>(worldPosition.y) / 32;

    m_World.renderPlacementPreview(*item, tileX, tileY, m_SelectedPlaceableDirection);
}

void Game::renderDebugOverlay() {
    const Vec2f playerPosition = m_World.getPlayerPosition();
    const int tileX = static_cast<int>(playerPosition.x) / 32;
    const int tileY = static_cast<int>(playerPosition.y) / 32;
    const ChunkManager& chunkManager = m_World.getChunkManager();
    const int chunkX = chunkManager.getCenterChunkX();
    const int chunkY = chunkManager.getCenterChunkY();
    const float zoom = m_World.getCamera().getZoom();

    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowBgAlpha(0.75f);
    ImGui::SetNextWindowPos(
        ImVec2(static_cast<float>(WINDOW_WIDTH) - 16.0f, static_cast<float>(WINDOW_HEIGHT) - 16.0f),
        ImGuiCond_Always,
        ImVec2(1.0f, 1.0f)
    );

    Vec2f mouseWorldPosition = getMouseWorldPosition();

    if (ImGui::Begin("DebugOverlay", nullptr, windowFlags)) {
        ImGui::Text("Grid: %d, %d", tileX, tileY);
        ImGui::Text("Chunk: %d, %d", chunkX, chunkY);
        ImGui::Text("Zoom: %.2f", zoom);
        ImGui::Text("Hovered Entity: %d", m_World.getHoveredMachine(mouseWorldPosition.x, mouseWorldPosition.y));
    }
    ImGui::End();
}

Vec2f Game::getMouseWorldPosition() {
    float mouseX;
    float mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    Camera2D& camera = m_World.getCamera();
    return Vec2f(mouseX / camera.getZoom() + camera.getX(), mouseY / camera.getZoom() + camera.getY());
}

void Game::rotateSelectedPlaceableDirection() {
    switch (m_SelectedPlaceableDirection) {
        case Direction::RIGHT:
            m_SelectedPlaceableDirection = Direction::DOWN;
            break;
        case Direction::DOWN:
            m_SelectedPlaceableDirection = Direction::LEFT;
            break;
        case Direction::LEFT:
            m_SelectedPlaceableDirection = Direction::UP;
            break;
        case Direction::UP:
        default:
            m_SelectedPlaceableDirection = Direction::RIGHT;
            break;
    }
}

