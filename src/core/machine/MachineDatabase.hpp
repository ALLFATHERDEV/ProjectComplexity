#pragma once

#include "MachineDefinition.hpp"

#include <string>
#include <unordered_map>

class MachineDatabase {
public:
    bool loadMachinesFromFolder(const std::string& folderPath);
    const MachineDefinition* getMachine(const std::string& uniqueName) const;

private:
    std::unordered_map<std::string, MachineDefinition> m_Machines;
};
