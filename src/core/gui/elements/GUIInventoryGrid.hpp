#pragma once
#include "../GUIDragContext.hpp"
#include "../GUIElement.hpp"
#include "../../inventory/InventoryGrid.hpp"

class GUIInventoryGrid : public GUIElement {
public:
    void setInventory(InventoryGrid* inventory) {
        m_Inventory = inventory;
    }
    void setSlotSize(float size) {
        m_SlotSize = size;
    }

    void setSpacing(float spacing) {
        m_Spacing = spacing;
    }

    void handleEvent(const SDL_Event &event) override {
        if (!m_Inventory)
            return;

        if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN)
            return;

        float mouseX = event.button.x;
        float mouseY = event.button.y;

        InventorySlot* clickedSlot = getSlotAt(mouseX, mouseY);
        if (!clickedSlot)
            return;

        if (event.button.button == SDL_BUTTON_LEFT) {
            handleLeftClick(clickedSlot);
            return;
        }

        if (event.button.button == SDL_BUTTON_RIGHT) {
            handleRightClick(clickedSlot);
            return;
        }

    }

    void render(Renderer* renderer) override {
        if (!m_Inventory)
            return;

        ItemStack hoveredStack;
        bool hasHoveredStack = false;
        for (int y = 0; y < m_Inventory->getHeight(); y++) {
            for (int x = 0; x < m_Inventory->getWidth(); x++) {

                InventorySlot* slot = m_Inventory->getSlot(x, y);

                SDL_FRect slotRect{
                    m_Position.x + x * (m_SlotSize + m_Spacing),
                    m_Position.y + y * (m_SlotSize + m_Spacing),
                    m_SlotSize,
                    m_SlotSize
                };

                renderer->drawFilledRect(slotRect, SDL_Color{55, 55, 55, 255});
                renderer->drawRect(slotRect, SDL_Color{120, 120, 120, 255});

                if (!slot || slot->isEmpty())
                    continue;

                drawStack(renderer, slot->stack, slotRect);

                if (isSlotHovered(slotRect) && !slot->isEmpty() && !m_DragContext) {
                    hoveredStack = slot->stack;
                    hasHoveredStack = true;
                }
            }
        }

        if (hasHoveredStack) {
            renderTooltip(renderer, hoveredStack);
        }

    }

    void setDragContext(GUIDragContext* dragContext) {
        m_DragContext = dragContext;
    }

private:

    void renderTooltip(Renderer* renderer, const ItemStack& stack) {
        if (stack.isEmpty() || !stack.item)
            return;

        float mouseX;
        float mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        SDL_FRect tooltipRect{mouseX + 12.0f, mouseY + 12.0f, 160.0f, 32.0f};
        renderer->drawFilledRect(tooltipRect, SDL_Color{20, 20, 20, 230});
        renderer->drawRect(tooltipRect, SDL_Color{180, 180, 180, 255});
        renderer->drawText(stack.item->displayName, tooltipRect.x + 8.0f, tooltipRect.y + 7.0f, SDL_Color{255, 255, 255, 255});
    }

    bool containsGridPoint(float mouseX, float mouseY) {
        float gridWidth =
            m_Inventory->getWidth() * m_SlotSize +
            (m_Inventory->getWidth() - 1) * m_Spacing;

        float gridHeight =
            m_Inventory->getHeight() * m_SlotSize +
            (m_Inventory->getHeight() - 1) * m_Spacing;

        return mouseX >= m_Position.x &&
               mouseY >= m_Position.y &&
               mouseX <= m_Position.x + gridWidth &&
               mouseY <= m_Position.y + gridHeight;
    }

    InventorySlot* getSlotAt(float mouseX, float mouseY) {
        if (!containsGridPoint(mouseX, mouseY))
            return nullptr;

        float localX = mouseX - m_Position.x;
        float localY = mouseY - m_Position.y;

        int slotX = static_cast<int>(localX / (m_SlotSize + m_Spacing));
        int slotY = static_cast<int>(localY / (m_SlotSize + m_Spacing));

        if (slotX < 0 ||slotY < 0 || slotX >= m_Inventory->getWidth() || slotY >= m_Inventory->getHeight())
            return nullptr;

        float slotStartX = slotX * (m_SlotSize + m_Spacing);
        float slotStartY = slotY * (m_SlotSize + m_Spacing);

        if (localX > slotStartX + m_SlotSize || localY > slotStartY + m_SlotSize)
            return nullptr;

        return m_Inventory->getSlot(slotX, slotY);
    }

    void drawStack(Renderer* renderer, const ItemStack& stack, const SDL_FRect& slotRect) {
        if (stack.isEmpty() || !stack.item || !stack.item->icon.texture)
            return;

        SDL_FRect iconRect{ slotRect.x + 4.0f, slotRect.y + 4.0f, slotRect.w - 8.0f, slotRect.h - 8.0f };
        renderer->draw( stack.item->icon.texture,  stack.item->icon.srcRect, iconRect  );

        if (stack.amount > 1) {
            renderer->drawText(std::to_string(stack.amount), slotRect.x + slotRect.w - 18.0f, slotRect.y + slotRect.h - 20.0f, SDL_Color{255, 255, 255, 255});
        }
    }

    void handleLeftClick(InventorySlot* clickedSlot) {
        if (!m_DragContext)
            return;

        if (!m_DragContext->isDragging && clickedSlot->isEmpty())
            return;

        if (!m_DragContext->isDragging) {
            m_DragContext->draggedStack = clickedSlot->stack;
            clickedSlot->stack.clear();
            m_DragContext->isDragging = true;
            return;
        }

        if (clickedSlot->isEmpty()) {
            clickedSlot->stack = m_DragContext->draggedStack;
            m_DragContext->clear();
            return;
        }

        if (clickedSlot->stack.item == m_DragContext->draggedStack.item) {
            int maxStack = clickedSlot->stack.item->maxStackSize;
            int space = maxStack - clickedSlot->stack.amount;

            if (space > 0) {
                int toMove = std::min(space, m_DragContext->draggedStack.amount);

                clickedSlot->stack.amount += toMove;
                m_DragContext->draggedStack.amount -= toMove;

                if (m_DragContext->draggedStack.amount <= 0) {
                    m_DragContext->clear();
                }

                return;
            }
        }

        std::swap(clickedSlot->stack, m_DragContext->draggedStack);

    }

    void handleRightClick(InventorySlot* clickedSlot) {
        if (!m_DragContext)
            return;

        if (!m_DragContext->isDragging && clickedSlot->isEmpty())
            return;

        if (!m_DragContext->isDragging)
        {
            int splitAmount = (clickedSlot->stack.amount + 1) / 2;

            m_DragContext->draggedStack.item = clickedSlot->stack.item;
            m_DragContext->draggedStack.amount = splitAmount;

            clickedSlot->stack.amount -= splitAmount;

            if (clickedSlot->stack.amount <= 0)
                clickedSlot->stack.clear();

            m_DragContext->isDragging = true;
            return;
        }

        if (m_DragContext->draggedStack.isEmpty()) {
            m_DragContext->clear();
            return;
        }

        if (clickedSlot->isEmpty())
        {
            clickedSlot->stack.item = m_DragContext->draggedStack.item;
            clickedSlot->stack.amount = 1;

            m_DragContext->draggedStack.amount--;

            if (m_DragContext->draggedStack.amount <= 0)
                m_DragContext->clear();

            return;
        }

        if (clickedSlot->stack.item == m_DragContext->draggedStack.item)
        {
            int maxStack = clickedSlot->stack.item->maxStackSize;

            if (clickedSlot->stack.amount < maxStack)
            {
                clickedSlot->stack.amount++;
                m_DragContext->draggedStack.amount--;

                if (m_DragContext->draggedStack.amount <= 0)
                    m_DragContext->clear();
            }
        }
    }

    bool isSlotHovered(const SDL_FRect& rect) {
        float mouseX;
        float mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        return mouseX >= rect.x && mouseY >= rect.y && mouseX <= rect.x + rect.w && mouseY <= rect.y + rect.h;
    }


private:
    InventoryGrid* m_Inventory = nullptr;

    float m_SlotSize = 48.0f;
    float m_Spacing = 4.0f;

    GUIDragContext* m_DragContext = nullptr;
};
