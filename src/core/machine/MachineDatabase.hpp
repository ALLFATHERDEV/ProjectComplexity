#pragma once

#include "MachineDefinition.hpp"

#include <memory>
#include <string>
#include <unordered_map>

class MachineDatabase {
public:
    bool loadMachinesFromFolder(const std::string& folderPath);
    const MachineDefinition* getMachine(const std::string& uniqueName) const;

    template<typename T>
    const T* getMachineAs(const std::string& uniqueName) const {
        return dynamic_cast<const T*>(getMachine(uniqueName));
    }

private:
    std::unordered_map<std::string, std::unique_ptr<MachineDefinition>> m_Machines;
    MachineType stringToMachineType(const std::string& str) const;
    FluidPortType stringToFluidPortType(const std::string& str) const;
    Direction stringToDirection(const std::string& str) const;
};
