#pragma once

#include <algorithm>
#include <cstdio>
#include <string>

#include "../GUIElement.hpp"
#include "../../entities/component/MachineFluidComponent.hpp"
#include "../../fluid/FluidDefinition.hpp"

class GUIFluidSlot : public GUIElement {
public:
    void setSlot(MachineFluidSlot* slot) {
        m_Slot = slot;
    }

    void setLabel(const std::string& label) {
        m_Label = label;
    }

    void setFillColor(SDL_Color color) {
        m_FillColor = color;
    }

    void render(Renderer* renderer) override {
        if (!isVisible()) {
            return;
        }

        renderer->drawFilledRect(m_Position, SDL_Color{55, 55, 55, 255});

        SDL_FRect innerRect{
            m_Position.x + 4.0f,
            m_Position.y + 4.0f,
            m_Position.w - 8.0f,
            m_Position.h - 8.0f
        };
        renderer->drawFilledRect(innerRect, SDL_Color{28, 28, 28, 255});

        if (m_Slot && m_Slot->capacity > 0.0f && m_Slot->storage.amount > 0.0f) {
            const float normalized = std::clamp(m_Slot->storage.amount / m_Slot->capacity, 0.0f, 1.0f);
            SDL_FRect fillRect = innerRect;
            fillRect.h = innerRect.h * normalized;
            fillRect.y = innerRect.y + innerRect.h - fillRect.h;
            renderer->drawFilledRect(fillRect, m_FillColor);
        }

        renderer->drawRect(m_Position, SDL_Color{120, 120, 120, 255});

        if (!m_Label.empty()) {
            renderer->drawText(m_Label, m_Position.x, m_Position.y - 18.0f, SDL_Color{220, 220, 220, 255});
        }

        if (m_Slot) {
            const std::string fluidShortName = m_Slot->storage.fluid ? makeShortLabel(m_Slot->storage.fluid->displayName) : "--";
            renderer->drawText(fluidShortName, m_Position.x + 8.0f, m_Position.y + 14.0f, SDL_Color{255, 255, 255, 255});
            renderer->drawText(formatAmount(m_Slot->storage.amount),
                               m_Position.x + 4.0f,
                               m_Position.y + m_Position.h - 18.0f,
                               SDL_Color{255, 255, 255, 255});
        }

        renderTooltip(renderer);
    }

private:
    void renderTooltip(Renderer* renderer) const {
        if (!m_Slot || !containsMouse()) {
            return;
        }

        float mouseX = 0.0f;
        float mouseY = 0.0f;
        SDL_GetMouseState(&mouseX, &mouseY);

        const std::string fluidName = m_Slot->storage.fluid ? m_Slot->storage.fluid->displayName : "Empty";
        const std::string slotName = m_Label.empty() ? "Fluid" : m_Label;
        const std::string amountText = formatAmount(m_Slot->storage.amount) + "/" +
                                       formatAmount(m_Slot->capacity);

        SDL_FRect tooltipRect{mouseX + 12.0f, mouseY + 12.0f, 180.0f, 48.0f};
        renderer->drawFilledRect(tooltipRect, SDL_Color{20, 20, 20, 230});
        renderer->drawRect(tooltipRect, SDL_Color{180, 180, 180, 255});
        renderer->drawText(slotName, tooltipRect.x + 8.0f, tooltipRect.y + 6.0f, SDL_Color{255, 255, 255, 255});
        renderer->drawText(fluidName + " " + amountText, tooltipRect.x + 8.0f, tooltipRect.y + 24.0f, SDL_Color{210, 210, 210, 255});
    }

    bool containsMouse() const {
        float mouseX = 0.0f;
        float mouseY = 0.0f;
        SDL_GetMouseState(&mouseX, &mouseY);
        return containsPoint(mouseX, mouseY);
    }

    std::string makeShortLabel(const std::string& name) const {
        if (name.size() <= 4) {
            return name;
        }

        return name.substr(0, 4);
    }

    std::string formatAmount(float value) const {
        char buffer[16]{};
        std::snprintf(buffer, sizeof(buffer), "%.1f", value);
        return buffer;
    }

    MachineFluidSlot* m_Slot = nullptr;
    std::string m_Label;
    SDL_Color m_FillColor{70, 150, 220, 255};
};
