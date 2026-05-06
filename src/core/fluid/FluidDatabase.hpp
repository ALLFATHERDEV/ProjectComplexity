#pragma once
#include <string>
#include <unordered_map>

#include "FluidDefinition.hpp"

class FluidDatabase {
public:
    bool loadFluidsFromFolder(const std::string& folderPath);
    const FluidDefinition* getFluid(const std::string& uniqueName) const;

private:
    std::unordered_map<std::string, FluidDefinition> m_Fluids;

};
