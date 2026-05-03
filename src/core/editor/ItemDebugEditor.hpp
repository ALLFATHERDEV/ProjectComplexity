#pragma once
#include "../world/World.hpp"

class ItemDebugEditor {
public:
    void setEnabled(bool enabled);
    void renderImGui(World& world);
    bool isEnabled() const;
    std::string toLowerCopy(const std::string& value);
    bool matchesSearch(const ItemDefinition& item, const char* searchBuffer);

private:
    bool m_Enabled = false;
    char m_SearchBuffer[256] = "";
};
