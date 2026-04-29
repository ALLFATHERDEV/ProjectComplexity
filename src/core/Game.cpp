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

    m_MachineGUI.create(m_GUISystem, &m_GUIDragContext);
    m_MachineGUI.bind(&m_World.getMachineInventories(),
                      &m_World.getCraftingMachines(),
                      &m_World.getRecipeDatabase(),
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
            m_TileMapEditor.update(event, m_World, m_World.getCamera());
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB) {
            m_MachineGUI.togglePlayerInventory();
        }
        m_GUISystem.handleEvent(event);

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

    m_TileMapEditor.renderImGui(m_World);

    ImGui::Render();

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer.getSDLRenderer());

    SDL_RenderPresent(m_Renderer.getSDLRenderer());
}

