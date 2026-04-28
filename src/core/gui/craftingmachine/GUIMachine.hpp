#pragma once
#include "../GUISystem.hpp"
#include "../../entities/ComponentStorage.hpp"
#include "../../entities/Entity.hpp"
#include "../../entities/component/CraftingMachineComponent.hpp"
#include "../../entities/component/MachineInventoryComponent.hpp"
#include "../../recipe/RecipeDatabase.hpp"
#include "../elements/GUIInventoryGrid.hpp"
#include "../elements/GUIPanel.hpp"
#include "../elements/GUIProgressBar.hpp"

class GUIMachine {
public:
    void create(GUISystem& guiSystem, GUIDragContext* dragContext);

    void open(Entity machine);
    void close();
    bool isOpen() const;

    void bind(ComponentStorage<MachineInventoryComponent>* machineInventories, ComponentStorage<CraftingMachineComponent>*craftingMachines, const RecipeDatabase* recipeDatabase);

    void update();

private:
    Entity m_SelectedMachine = 0;
    bool m_Open = false;

    const RecipeDatabase* m_RecipeDatabase = nullptr;

    ComponentStorage<MachineInventoryComponent>* m_MachineInventories = nullptr;
    ComponentStorage<CraftingMachineComponent>* m_CraftingMachines = nullptr;

    GUIPanel* m_Panel = nullptr;
    GUIInventoryGrid* m_InputGrid = nullptr;
    GUIInventoryGrid* m_OutputGrid = nullptr;
    GUIProgressBar* m_ProgressBar = nullptr;

};
