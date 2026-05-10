#include "Game.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "Logger.hpp"
#include "screen/MainMenuScreen.hpp"

int Game::WINDOW_WIDTH = 1800;
int Game::WINDOW_HEIGHT = 1200;

Game::Game()
    : m_ScreenContext{*this, m_Renderer, nullptr} {
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

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(m_Window, m_Renderer.getSDLRenderer());
    ImGui_ImplSDLRenderer3_Init(m_Renderer.getSDLRenderer());
    m_ScreenContext.window = m_Window;
    changeScreen(std::make_unique<MainMenuScreen>(m_ScreenContext));

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
        Uint64 currentTicks = SDL_GetTicks();
        Uint64 deltaTimeMs = currentTicks - previousTicks;
        previousTicks = currentTicks;
        float deltaTime = static_cast<float>(deltaTimeMs) / 1000.0f;

        events();
        if (m_CurrentScreen) {
            m_CurrentScreen->update(deltaTime);
        }
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

        if (m_CurrentScreen) {
            m_CurrentScreen->handleEvent(event);
        }
    }
}

void Game::render() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    SDL_RenderClear(m_Renderer.getSDLRenderer());
    if (m_CurrentScreen) {
        m_CurrentScreen->render();
    }
    m_Renderer.flushSpriteQueue();

    ImGui::Render();

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer.getSDLRenderer());

    SDL_RenderPresent(m_Renderer.getSDLRenderer());
}

void Game::changeScreen(std::unique_ptr<IScreen> screen) {
    if (m_CurrentScreen) {
        m_CurrentScreen->onExit();
    }

    m_CurrentScreen = std::move(screen);

    if (m_CurrentScreen) {
        m_CurrentScreen->onEnter();
    }
}

