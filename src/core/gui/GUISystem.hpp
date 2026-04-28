#pragma once

#include <vector>
#include <memory>

#include "GUIElement.hpp"

class GUISystem {
public:
    template<typename T, typename... Args>
    T* addElement(Args&&... args) {
        auto element = std::make_unique<T>(std::forward<Args>(args)...);
        T* rawPtr = element.get();

        m_Elements.push_back(std::move(element));
        return rawPtr;
    }

    void update(float deltaTime) {
        for (auto& element : m_Elements) {
            if (element->isVisible())
                element->update(deltaTime);
        }
    }

    void render(Renderer* renderer) {
        for (auto& element : m_Elements) {
            if (element->isVisible())
                element->render(renderer);
        }
    }

    void handleEvent(const SDL_Event& event) {
        for (auto& element : m_Elements) {
            element->handleEvent(event);
        }
    }

private:
    std::vector<std::unique_ptr<GUIElement>> m_Elements;
};
