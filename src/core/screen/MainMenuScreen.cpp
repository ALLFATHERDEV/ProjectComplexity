#include "MainMenuScreen.hpp"

#include "imgui.h"
#include "../Game.hpp"
#include "WorldScreen.hpp"

MainMenuScreen::MainMenuScreen(ScreenContext& context)
    : m_Context(context) {
}

void MainMenuScreen::update(float deltaTime) {
    (void)deltaTime;
}

void MainMenuScreen::render() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 windowSize(260.0f, 120.0f);
    const ImVec2 windowPos(
        viewport->WorkPos.x + (viewport->WorkSize.x - windowSize.x) * 0.5f,
        viewport->WorkPos.y + (viewport->WorkSize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize;

    if (ImGui::Begin("Main Menu", nullptr, windowFlags)) {
        if (ImGui::Button("Start", ImVec2(-1.0f, 0.0f))) {
            m_Context.game.changeScreen(std::make_unique<WorldScreen>(m_Context));
        }

        if (ImGui::Button("Exit", ImVec2(-1.0f, 0.0f))) {
            m_Context.game.requestQuit();
        }
    }
    ImGui::End();
}

void MainMenuScreen::handleEvent(SDL_Event& event) {
    (void)event;
}
