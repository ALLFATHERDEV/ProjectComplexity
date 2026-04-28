#include "GUIMachine.hpp"

void GUIMachine::create(GUISystem &guiSystem, GUIDragContext* dragContext) {

    m_Panel = guiSystem.addElement<GUIPanel>();
    m_Panel->setPosition(360.0f, 120.0f);
    m_Panel->setSize(560.0f, 360.0f);
    m_Panel->setColor({20, 20, 20, 235});
    m_Panel->setVisible(false);

    m_InputGrid = guiSystem.addElement<GUIInventoryGrid>();
    m_InputGrid->setPosition(410.0f, 190.0f);
    m_InputGrid->setSlotSize(52.0f);
    m_InputGrid->setSpacing(6.0f);
    m_InputGrid->setDragContext(dragContext);
    m_InputGrid->setVisible(false);

    m_OutputGrid = guiSystem.addElement<GUIInventoryGrid>();
    m_OutputGrid->setPosition(760.0f, 190.0f);
    m_OutputGrid->setSlotSize(52.0f);
    m_OutputGrid->setSpacing(6.0f);
    m_OutputGrid->setDragContext(dragContext);
    m_OutputGrid->setVisible(false);

    m_ProgressBar = guiSystem.addElement<GUIProgressBar>();
    m_ProgressBar->setPosition(410.0f, 330.0f);
    m_ProgressBar->setSize(460.0f, 24.0f);
    m_ProgressBar->setVisible(false);
}

void GUIMachine::open(Entity machine) {
    m_SelectedMachine = machine;
    m_Open = true;

    if (m_Panel)
        m_Panel->setVisible(true);

    if (!m_MachineInventories)
        return;

    auto* inventory = m_MachineInventories->get(machine);
    if (!inventory)
        return;

    if (m_InputGrid) {
        m_InputGrid->setInventory(&inventory->inputInventory);
        m_InputGrid->setVisible(true);
    }

    if (m_OutputGrid) {
        m_OutputGrid->setInventory(&inventory->outputInventory);
        m_OutputGrid->setVisible(true);
    }

    if (m_ProgressBar) {
        m_ProgressBar->setVisible(true);
    }
}

void GUIMachine::close() {
    m_Open = false;
    m_SelectedMachine = 0;

    if (m_Panel)
        m_Panel->setVisible(false);

    if (m_InputGrid)
        m_InputGrid->setVisible(false);

    if (m_OutputGrid)
        m_OutputGrid->setVisible(false);

    if (m_ProgressBar)
        m_ProgressBar->setVisible(false);


}

bool GUIMachine::isOpen() const {
    return m_Open;
}

void GUIMachine::bind(ComponentStorage<MachineInventoryComponent> *machineInventories, ComponentStorage<CraftingMachineComponent> *craftingMachines, const RecipeDatabase* recipeDatabase) {
    m_CraftingMachines = craftingMachines;
    m_MachineInventories = machineInventories;
    m_RecipeDatabase = recipeDatabase;
}

void GUIMachine::update() {
    if (!m_Open || !m_MachineInventories)
        return;

    auto* inventory = m_MachineInventories->get(m_SelectedMachine);

    if (!inventory) {
        close();
        return;
    }

    if (m_InputGrid)
        m_InputGrid->setInventory(&inventory->inputInventory);

    if (m_OutputGrid)
        m_OutputGrid->setInventory(&inventory->outputInventory);

    if (m_ProgressBar && m_CraftingMachines) {
        auto* machine = m_CraftingMachines->get(m_SelectedMachine);
        if (machine) {
            const RecipeDefinition* recipe = m_RecipeDatabase->getRecipe(machine->currentRecipeName);
            if (recipe && recipe->craftTime > 0.0f) {
                m_ProgressBar->setValue(machine->progress / recipe->craftTime);
            } else {
                m_ProgressBar->setValue(0.0f);
            }
        }
    }
}
