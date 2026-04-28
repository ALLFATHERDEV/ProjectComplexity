#pragma once
#include "../GUIDragContext.hpp"
#include "../GUIElement.hpp"


class GUIDragPreview : public GUIElement {
public:
    void setDragContext(GUIDragContext* context) {
        m_DragContext = context;
    }

    void render(Renderer* renderer) override {
        if (!m_DragContext)
            return;

        if (!m_DragContext->isDragging || m_DragContext->draggedStack.isEmpty())
            return;

        float mouseX;
        float mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        SDL_FRect rect{mouseX - 24.0f, mouseY - 24.0f, 48.0f, 48.0f
        };

        const ItemStack& stack = m_DragContext->draggedStack;

        renderer->draw(stack.item->icon.texture,stack.item->icon.srcRect, rect);

        if (stack.amount > 1) {
            renderer->drawText(std::to_string(stack.amount), rect.x + rect.w - 18.0f,rect.y + rect.h - 20.0f,  SDL_Color{255, 255, 255, 255}
            );
        }
    }

private:
    GUIDragContext* m_DragContext = nullptr;
};
