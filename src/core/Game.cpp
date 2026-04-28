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

    m_InventoryPanel = m_GUISystem.addElement<GUIPanel>();
    m_InventoryPanel->setPosition(1040.0f, 120.0f);
    m_InventoryPanel->setSize(600.0f, 400.0f);
    m_InventoryPanel->setColor({20, 20, 20, 230});
    m_InventoryPanel->setVisible(false);

    m_PlayerInventoryGrid = m_GUISystem.addElement<GUIInventoryGrid>();
    m_PlayerInventoryGrid->setPosition(1080.0f, 170.0f);
    m_PlayerInventoryGrid->setSize(500.0f, 300.0f);
    m_PlayerInventoryGrid->setSlotSize(48.0f);
    m_PlayerInventoryGrid->setSpacing(6.0f);
    m_PlayerInventoryGrid->setDragContext(&m_GUIDragContext);
    m_PlayerInventoryGrid->setVisible(false);

    auto* playerInventory = m_World.getInventories().get(m_World.getPlayer());
    if (playerInventory)
        m_PlayerInventoryGrid->setInventory(&playerInventory->inventory);

    m_MachineGUI.create(m_GUISystem, &m_GUIDragContext);
    m_MachineGUI.bind(&m_World.getMachineInventories(), &m_World.getCraftingMachines(), &m_World.getRecipeDatabase());

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
        float deltaTime = deltaTimeMs / 1000.0f;

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

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            m_TileMapEditor.update(event, m_World.getTileMap(), m_World.getTileMapAtlas(), m_World.getCamera());
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB) {
            bool open = !m_InventoryPanel->isVisible();

            m_InventoryPanel->setVisible(open);
            m_PlayerInventoryGrid->setVisible(open);
        }
        m_GUISystem.handleEvent(event);

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            if (m_MachineGUI.isOpen()) {
                m_MachineGUI.close();
            }
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_E) {
            auto interactedEntity = m_World.tryInteract(event);
            if (interactedEntity.has_value()) {
                if (m_World.getMachineInventories().get(interactedEntity.value())) {
                    m_MachineGUI.open(interactedEntity.value());
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
    m_GUISystem.render(&m_Renderer);

    m_TileMapEditor.renderImGui(m_World.getTileMap(), m_World.getTileMapAtlas());

    ImGui::Render();

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer.getSDLRenderer());

    SDL_RenderPresent(m_Renderer.getSDLRenderer());
}

