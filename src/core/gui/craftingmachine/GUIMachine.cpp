#include "GUIMachine.hpp"

void GUIMachine::create(GUISystem &guiSystem, GUIDragContext* dragContext) {
    m_GUISystem = &guiSystem;

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

    m_PlayerInventoryPanel = guiSystem.addElement<GUIPanel>();
    m_PlayerInventoryPanel->setPosition(960.0f, 120.0f);
    m_PlayerInventoryPanel->setSize(600.0f, 360.0f);
    m_PlayerInventoryPanel->setColor({20, 20, 20, 235});
    m_PlayerInventoryPanel->setVisible(false);

    m_PlayerInventoryGrid = guiSystem.addElement<GUIInventoryGrid>();
    m_PlayerInventoryGrid->setPosition(1000.0f, 180.0f);
    m_PlayerInventoryGrid->setSlotSize(48.0f);
    m_PlayerInventoryGrid->setSpacing(6.0f);
    m_PlayerInventoryGrid->setDragContext(dragContext);
    m_PlayerInventoryGrid->setVisible(false);
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

    showPlayerInventory();

    if (m_GUISystem)
        rebuildRecipeButtons(*m_GUISystem);

    updateRecipeButtons();
}

void GUIMachine::openPlayerInventory() {
    if (m_Open) {
        return;
    }

    m_PlayerInventoryOpen = true;
    showPlayerInventory();
}

void GUIMachine::togglePlayerInventory() {
    if (m_Open) {
        close();
        return;
    }

    if (m_PlayerInventoryOpen) {
        close();
        return;
    }

    openPlayerInventory();
}

void GUIMachine::close() {
    m_Open = false;
    m_PlayerInventoryOpen = false;
    m_SelectedMachine = 0;

    if (m_Panel)
        m_Panel->setVisible(false);

    if (m_InputGrid)
        m_InputGrid->setVisible(false);

    if (m_OutputGrid)
        m_OutputGrid->setVisible(false);

    if (m_ProgressBar)
        m_ProgressBar->setVisible(false);

    hidePlayerInventory();

    for (auto* button : m_RecipeButtons) {
        button->setVisible(false);
    }
}

bool GUIMachine::isOpen() const {
    return m_Open;
}

bool GUIMachine::isPlayerInventoryOpen() const {
    return m_PlayerInventoryOpen || m_Open;
}

void GUIMachine::bind(ComponentStorage<MachineInventoryComponent>* machineInventories,
                      ComponentStorage<CraftingMachineComponent>* craftingMachines,
                      const RecipeDatabase* recipeDatabase,
                      ComponentStorage<InventoryComponent>* inventories,
                      Entity player) {
    m_CraftingMachines = craftingMachines;
    m_MachineInventories = machineInventories;
    m_RecipeDatabase = recipeDatabase;
    m_Inventories = inventories;
    m_Player = player;
}

void GUIMachine::update() {
    if (!m_Open && !m_PlayerInventoryOpen)
        return;

    if (m_PlayerInventoryGrid && m_Inventories) {
        auto* playerInventory = m_Inventories->get(m_Player);
        if (playerInventory) {
            m_PlayerInventoryGrid->setInventory(&playerInventory->inventory);
        }
    }

    if (!m_Open)
        return;

    if (!m_MachineInventories)
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

    updateRecipeButtons();
}

void GUIMachine::updateRecipeButtons() {
    for (auto* button : m_RecipeButtons) {
        button->setVisible(false);
    }

    if (!m_Open || !m_CraftingMachines || !m_RecipeDatabase)
        return;

    auto* machine = m_CraftingMachines->get(m_SelectedMachine);
    if (!machine)
        return;

    for (size_t i = 0; i < machine->availableRecipes.size() && i < m_RecipeButtons.size(); i++) {
        const std::string recipeName = machine->availableRecipes[i];

        const RecipeDefinition* recipe =
            m_RecipeDatabase->getRecipe(recipeName);

        if (!recipe)
            continue;

        GUIButton* button = m_RecipeButtons[i];

        std::string label = recipe->displayName;

        if (machine->currentRecipeName == recipeName) {
            label = "> " + recipe->displayName ;
        }

        button->setText(label);
        button->setVisible(true);

        button->setOnClick([this, recipeName]() {
            auto* selectedMachine = m_CraftingMachines->get(m_SelectedMachine);
            if (!selectedMachine)
                return;

            if (selectedMachine->currentRecipeName != recipeName) {
                selectedMachine->currentRecipeName = recipeName;
                selectedMachine->progress = 0.0f;
                selectedMachine->isCrafting = false;
            }

            updateRecipeButtons();
        });
    }
}

void GUIMachine::rebuildRecipeButtons(GUISystem &guiSystem) {
    for (int i = 0; i < 8; i++) {
        auto* button = guiSystem.addElement<GUIButton>();

        button->setPosition(410.0f, 370.0f + i * 34.0f);
        button->setSize(220.0f, 28.0f);
        button->setVisible(false);

        m_RecipeButtons.push_back(button);
    }
}

void GUIMachine::showPlayerInventory() {
    auto* playerInventory = m_Inventories ? m_Inventories->get(m_Player) : nullptr;
    if (!playerInventory) {
        return;
    }

    if (m_PlayerInventoryPanel) {
        m_PlayerInventoryPanel->setVisible(true);
    }

    if (m_PlayerInventoryGrid) {
        m_PlayerInventoryGrid->setInventory(&playerInventory->inventory);
        m_PlayerInventoryGrid->setVisible(true);
    }
}

void GUIMachine::hidePlayerInventory() {
    if (m_PlayerInventoryPanel) {
        m_PlayerInventoryPanel->setVisible(false);
    }

    if (m_PlayerInventoryGrid) {
        m_PlayerInventoryGrid->setVisible(false);
    }
}
