#include "GUIMachine.hpp"
#include <algorithm>

void GUIMachine::create(GUISystem &guiSystem, GUIDragContext* dragContext) {
    m_GUISystem = &guiSystem;

    m_Panel = guiSystem.addElement<GUIPanel>();
    m_Panel->setPosition(360.0f, 120.0f);
    m_Panel->setSize(560.0f, 360.0f);
    m_Panel->setColor({20, 20, 20, 235});
    m_Panel->setVisible(false);

    m_FuelGrid = guiSystem.addElement<GUIInventoryGrid>();
    m_FuelGrid->setPosition(410.0f, 130.0f);
    m_FuelGrid->setSlotSize(52.0f);
    m_FuelGrid->setSpacing(6.0f);
    m_FuelGrid->setDragContext(dragContext);
    m_FuelGrid->setShiftClickFn([this](InventorySlot& slot) { return handleMachineFuelShiftClick(slot); });
    m_FuelGrid->setVisible(false);

    m_FuelProgressBar = guiSystem.addElement<GUIProgressBar>();
    m_FuelProgressBar->setPosition(470.0f, 146.0f);
    m_FuelProgressBar->setSize(180.0f, 18.0f);
    m_FuelProgressBar->setBackgroundColor({45, 32, 22, 255});
    m_FuelProgressBar->setFillColor({220, 150, 60, 255});
    m_FuelProgressBar->setVisible(false);

    m_InputGrid = guiSystem.addElement<GUIInventoryGrid>();
    m_InputGrid->setPosition(410.0f, 190.0f);
    m_InputGrid->setSlotSize(52.0f);
    m_InputGrid->setSpacing(6.0f);
    m_InputGrid->setDragContext(dragContext);
    m_InputGrid->setAcceptStackFn([this](const ItemStack& stack) { return canAcceptInputStack(stack); });
    m_InputGrid->setAcceptStackAtSlotFn([this](int slotX, int slotY, const ItemStack& stack) { return canAcceptInputStackAtSlot(slotX, slotY, stack); });
    m_InputGrid->setSlotBackgroundItemFn([this](int slotX, int slotY) { return getInputSlotBackgroundItem(slotX, slotY); });
    m_InputGrid->setShiftClickFn([this](InventorySlot& slot) { return handleMachineInputShiftClick(slot); });
    m_InputGrid->setVisible(false);

    m_OutputGrid = guiSystem.addElement<GUIInventoryGrid>();
    m_OutputGrid->setPosition(760.0f, 190.0f);
    m_OutputGrid->setSlotSize(52.0f);
    m_OutputGrid->setSpacing(6.0f);
    m_OutputGrid->setDragContext(dragContext);
    m_OutputGrid->setAcceptStackFn([this](const ItemStack& stack) { return canAcceptOutputStack(stack); });
    m_OutputGrid->setAcceptStackAtSlotFn([this](int slotX, int slotY, const ItemStack& stack) { return canAcceptOutputStackAtSlot(slotX, slotY, stack); });
    m_OutputGrid->setSlotBackgroundItemFn([this](int slotX, int slotY) { return getOutputSlotBackgroundItem(slotX, slotY); });
    m_OutputGrid->setShiftClickFn([this](InventorySlot& slot) { return handleMachineOutputShiftClick(slot); });
    m_OutputGrid->setVisible(false);

    m_StorageGrid = guiSystem.addElement<GUIInventoryGrid>();
    m_StorageGrid->setPosition(410.0f, 190.0f);
    m_StorageGrid->setSlotSize(52.0f);
    m_StorageGrid->setSpacing(6.0f);
    m_StorageGrid->setDragContext(dragContext);
    m_StorageGrid->setShiftClickFn([this](InventorySlot& slot) { return handleStorageShiftClick(slot); });
    m_StorageGrid->setVisible(false);

    m_ProgressBar = guiSystem.addElement<GUIProgressBar>();
    m_ProgressBar->setPosition(410.0f, 330.0f);
    m_ProgressBar->setSize(460.0f, 24.0f);
    m_ProgressBar->setVisible(false);

    m_MinerInfoText = guiSystem.addElement<GUIText>();
    m_MinerInfoText->setPosition(410.0f, 200.0f);
    m_MinerInfoText->setVisible(false);

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
    m_PlayerInventoryGrid->setShiftClickFn([this](InventorySlot& slot) { return handlePlayerShiftClick(slot); });
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

    const bool craftingMachine = isCraftingMachine(machine);

    if (m_InputGrid) {
        m_InputGrid->setInventory(&inventory->inputInventory);
        m_InputGrid->setVisible(craftingMachine);
    }

    if (m_FuelGrid) {
        m_FuelGrid->setInventory(&inventory->fuelInventory);
        m_FuelGrid->setVisible(true);
    }

    if (m_FuelProgressBar) {
        m_FuelProgressBar->setVisible(true);
    }

    if (m_OutputGrid) {
        m_OutputGrid->setInventory(&inventory->outputInventory);
        m_OutputGrid->setVisible(true);
    }

    if (m_StorageGrid) {
        m_StorageGrid->setVisible(false);
    }

    if (m_ProgressBar) {
        m_ProgressBar->setVisible(true);
    }

    if (m_MinerInfoText) {
        m_MinerInfoText->setVisible(isMiner(machine));
    }

    showPlayerInventory();

    if (m_GUISystem)
        rebuildRecipeButtons(*m_GUISystem);

    updateRecipeButtons();
}

void GUIMachine::openStorage(Entity storage) {
    m_SelectedMachine = storage;
    m_Open = true;

    if (m_Panel) {
        m_Panel->setVisible(true);
    }

    if (m_FuelGrid) {
        m_FuelGrid->setVisible(false);
    }

    if (m_FuelProgressBar) {
        m_FuelProgressBar->setVisible(false);
    }

    if (m_InputGrid) {
        m_InputGrid->setVisible(false);
    }

    if (m_OutputGrid) {
        m_OutputGrid->setVisible(false);
    }

    if (m_ProgressBar) {
        m_ProgressBar->setVisible(false);
    }

    if (m_MinerInfoText) {
        m_MinerInfoText->setVisible(false);
    }

    if (m_StorageGrid) {
        auto* inventory = m_Inventories ? m_Inventories->get(storage) : nullptr;
        m_StorageGrid->setInventory(inventory ? &inventory->inventory : nullptr);
        m_StorageGrid->setVisible(inventory != nullptr);
    }

    for (auto* button : m_RecipeButtons) {
        button->setVisible(false);
    }

    showPlayerInventory();
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

    if (m_FuelGrid)
        m_FuelGrid->setVisible(false);

    if (m_FuelProgressBar)
        m_FuelProgressBar->setVisible(false);

    if (m_OutputGrid)
        m_OutputGrid->setVisible(false);

    if (m_StorageGrid)
        m_StorageGrid->setVisible(false);

    if (m_ProgressBar)
        m_ProgressBar->setVisible(false);

    if (m_MinerInfoText)
        m_MinerInfoText->setVisible(false);

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
                      ComponentStorage<MinerComponent>* miners,
                      const RecipeDatabase* recipeDatabase,
                      const ItemDatabase* itemDatabase,
                      ComponentStorage<InventoryComponent>* inventories,
                      Entity player) {
    m_CraftingMachines = craftingMachines;
    m_Miners = miners;
    m_MachineInventories = machineInventories;
    m_RecipeDatabase = recipeDatabase;
    m_ItemDatabase = itemDatabase;
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
    {
        if (!isStorageContainer(m_SelectedMachine)) {
            close();
        }
        return;
    }

    if (isStorageContainer(m_SelectedMachine)) {
        if (m_StorageGrid && m_Inventories) {
            auto* storageInventory = m_Inventories->get(m_SelectedMachine);
            if (!storageInventory) {
                close();
                return;
            }

            m_StorageGrid->setInventory(&storageInventory->inventory);
            m_StorageGrid->setVisible(true);
        }

        return;
    }

    auto* inventory = m_MachineInventories->get(m_SelectedMachine);

    if (!inventory) {
        close();
        return;
    }

    if (m_FuelGrid)
        m_FuelGrid->setInventory(&inventory->fuelInventory);

    if (m_FuelProgressBar) {
        bool showFuelProgress = false;

        if (auto* machine = m_CraftingMachines ? m_CraftingMachines->get(m_SelectedMachine) : nullptr) {
            if (machine->requiresFuel) {
                m_FuelProgressBar->setValue(machine->currentFuelCapacity > 0.0f ? machine->fuelRemaining / machine->currentFuelCapacity : 0.0f);
                showFuelProgress = true;
            }
        } else if (auto* miner = m_Miners ? m_Miners->get(m_SelectedMachine) : nullptr) {
            if (miner->requiresFuel) {
                m_FuelProgressBar->setValue(miner->currentFuelCapacity > 0.0f ? miner->fuelRemaining / miner->currentFuelCapacity : 0.0f);
                showFuelProgress = true;
            }
        }

        m_FuelProgressBar->setVisible(showFuelProgress);
    }

    if (m_InputGrid) {
        m_InputGrid->setInventory(&inventory->inputInventory);
        m_InputGrid->setVisible(isCraftingMachine(m_SelectedMachine));
    }

    if (m_OutputGrid)
        m_OutputGrid->setInventory(&inventory->outputInventory);

    if (m_ProgressBar) {
        if (auto* machine = m_CraftingMachines ? m_CraftingMachines->get(m_SelectedMachine) : nullptr) {
            const RecipeDefinition* recipe = m_RecipeDatabase->getRecipe(machine->currentRecipeName);
            if (recipe && recipe->craftTime > 0.0f) {
                m_ProgressBar->setValue(machine->progress / recipe->craftTime);
            } else {
                m_ProgressBar->setValue(0.0f);
            }
            m_ProgressBar->setVisible(true);
        } else if (auto* miner = m_Miners ? m_Miners->get(m_SelectedMachine) : nullptr) {
            m_ProgressBar->setValue(miner->miningProgress);
            m_ProgressBar->setVisible(true);
        } else {
            m_ProgressBar->setValue(0.0f);
            m_ProgressBar->setVisible(false);
        }
    }

    if (m_MinerInfoText) {
        if (auto* miner = m_Miners ? m_Miners->get(m_SelectedMachine) : nullptr) {
            std::string minedItemLabel = "Mining: None";
            if (!miner->currentMinedItemName.empty()) {
                if (const ItemDefinition* item = m_ItemDatabase ? m_ItemDatabase->getItem(miner->currentMinedItemName) : nullptr) {
                    minedItemLabel = "Mining: " + item->displayName + " (" + miner->currentOrePatchQuality + ")";
                } else {
                    minedItemLabel = "Mining: " + miner->currentMinedItemName + " (" + miner->currentOrePatchQuality + ")";
                }
            }

            m_MinerInfoText->setText(minedItemLabel);
            m_MinerInfoText->setVisible(true);
        } else {
            m_MinerInfoText->setVisible(false);
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

            const RecipeDefinition* nextRecipe = m_RecipeDatabase ? m_RecipeDatabase->getRecipe(recipeName) : nullptr;
            if (!nextRecipe)
                return;

            if (selectedMachine->currentRecipeName != recipeName) {
                if (!tryApplyRecipeLayout(*nextRecipe)) {
                    return;
                }

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

bool GUIMachine::isCraftingMachine(Entity machine) const {
    return m_CraftingMachines && m_CraftingMachines->get(machine);
}

bool GUIMachine::isMiner(Entity machine) const {
    return m_Miners && m_Miners->get(machine);
}

bool GUIMachine::isStorageContainer(Entity entity) const {
    if (!m_Inventories || entity == m_Player) {
        return false;
    }

    return m_Inventories->get(entity) != nullptr &&
           (!m_MachineInventories || m_MachineInventories->get(entity) == nullptr);
}

bool GUIMachine::transferSlotToInventory(InventorySlot& sourceSlot,
                                         InventoryGrid& targetInventory,
                                         const GUIInventoryGrid::AcceptStackFn* acceptStackFn) const {
    if (sourceSlot.isEmpty() || !sourceSlot.stack.item) {
        return false;
    }

    const ItemDefinition* item = sourceSlot.stack.item;
    int moved = 0;

    while (sourceSlot.stack.amount > 0) {
        if (acceptStackFn) {
            const ItemStack singleItem{item, 1};
            if (!(*acceptStackFn)(singleItem)) {
                break;
            }
        }

        if (!targetInventory.addItem(item, 1)) {
            break;
        }

        sourceSlot.stack.amount--;
        moved++;
    }

    if (sourceSlot.stack.amount <= 0) {
        sourceSlot.stack.clear();
    }

    return moved > 0;
}

bool GUIMachine::transferSlotToReservedRecipeInputs(InventorySlot& sourceSlot, InventoryGrid& targetInventory) const {
    if (sourceSlot.isEmpty() || !sourceSlot.stack.item) {
        return false;
    }

    const RecipeDefinition* recipe = getSelectedRecipe();
    if (!recipe) {
        return false;
    }

    int moved = 0;
    for (size_t i = 0; i < recipe->inputs.size() && sourceSlot.stack.amount > 0; i++) {
        const auto& ingredient = recipe->inputs[i];
        if (ingredient.itemName != sourceSlot.stack.item->uniqueName) {
            continue;
        }

        InventorySlot* targetSlot = targetInventory.getSlot(static_cast<int>(i), 0);
        if (!targetSlot) {
            continue;
        }

        if (targetSlot->isEmpty()) {
            targetSlot->stack.item = sourceSlot.stack.item;
            targetSlot->stack.amount = sourceSlot.stack.amount;
            sourceSlot.stack.amount = 0;
            moved++;
            continue;
        }

        if (targetSlot->stack.item != sourceSlot.stack.item) {
            continue;
        }

        if (targetSlot->stack.amount >= targetSlot->stack.item->maxStackSize) {
            continue;
        }

        targetSlot->stack.amount++;
        sourceSlot.stack.amount--;
        moved++;
    }

    if (sourceSlot.stack.amount <= 0) {
        sourceSlot.stack.clear();
    }

    return moved > 0;
}

bool GUIMachine::handlePlayerShiftClick(InventorySlot& sourceSlot) const {
    if (!m_Open || sourceSlot.isEmpty()) {
        return false;
    }

    if (isStorageContainer(m_SelectedMachine)) {
        auto* storageInventory = m_Inventories->get(m_SelectedMachine);
        return storageInventory && transferSlotToInventory(sourceSlot, storageInventory->inventory);
    }

    auto* machineInventory = m_MachineInventories ? m_MachineInventories->get(m_SelectedMachine) : nullptr;
    if (!machineInventory) {
        return false;
    }

    if (isCraftingMachine(m_SelectedMachine)) {
        if (transferSlotToReservedRecipeInputs(sourceSlot, machineInventory->inputInventory)) {
            return true;
        }
    }

    const GUIInventoryGrid::AcceptStackFn fuelAcceptFn = [](const ItemStack& stack) {
        return !stack.isEmpty() && stack.item && stack.item->fuelValue > 0.0f;
    };

    return transferSlotToInventory(sourceSlot, machineInventory->fuelInventory, &fuelAcceptFn);
}

bool GUIMachine::handleMachineInputShiftClick(InventorySlot& sourceSlot) const {
    auto* playerInventory = m_Inventories ? m_Inventories->get(m_Player) : nullptr;
    return playerInventory && transferSlotToInventory(sourceSlot, playerInventory->inventory);
}

bool GUIMachine::handleMachineFuelShiftClick(InventorySlot& sourceSlot) const {
    auto* playerInventory = m_Inventories ? m_Inventories->get(m_Player) : nullptr;
    return playerInventory && transferSlotToInventory(sourceSlot, playerInventory->inventory);
}

bool GUIMachine::handleMachineOutputShiftClick(InventorySlot& sourceSlot) const {
    auto* playerInventory = m_Inventories ? m_Inventories->get(m_Player) : nullptr;
    return playerInventory && transferSlotToInventory(sourceSlot, playerInventory->inventory);
}

bool GUIMachine::handleStorageShiftClick(InventorySlot& sourceSlot) const {
    auto* playerInventory = m_Inventories ? m_Inventories->get(m_Player) : nullptr;
    return playerInventory && transferSlotToInventory(sourceSlot, playerInventory->inventory);
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

bool GUIMachine::canAcceptInputStack(const ItemStack& stack) const {
    if (stack.isEmpty() || !stack.item || !m_CraftingMachines || !m_RecipeDatabase) {
        return false;
    }

    auto* machine = m_CraftingMachines->get(m_SelectedMachine);
    if (!machine) {
        return false;
    }

    const RecipeDefinition* recipe = m_RecipeDatabase->getRecipe(machine->currentRecipeName);
    if (!recipe) {
        return false;
    }

    for (const auto& ingredient : recipe->inputs) {
        if (ingredient.itemName == stack.item->uniqueName) {
            return true;
        }
    }

    return false;
}

bool GUIMachine::canAcceptInputStackAtSlot(int slotX, int slotY, const ItemStack& stack) const {
    if (slotY != 0 || slotX < 0 || stack.isEmpty() || !stack.item) {
        return false;
    }

    const RecipeDefinition* recipe = getSelectedRecipe();
    if (!recipe) {
        return false;
    }

    if (slotX >= static_cast<int>(recipe->inputs.size())) {
        return false;
    }

    return recipe->inputs[slotX].itemName == stack.item->uniqueName;
}

const RecipeDefinition* GUIMachine::getSelectedRecipe() const {
    if (!m_CraftingMachines || !m_RecipeDatabase) {
        return nullptr;
    }

    auto* machine = m_CraftingMachines->get(m_SelectedMachine);
    if (!machine) {
        return nullptr;
    }

    return m_RecipeDatabase->getRecipe(machine->currentRecipeName);
}

const ItemDefinition* GUIMachine::getInputSlotBackgroundItem(int slotX, int slotY) const {
    if (slotY != 0 || slotX < 0 || !m_ItemDatabase) {
        return nullptr;
    }

    const RecipeDefinition* recipe = getSelectedRecipe();
    if (!recipe) {
        return nullptr;
    }

    if (slotX >= static_cast<int>(recipe->inputs.size())) {
        return nullptr;
    }

    return m_ItemDatabase->getItem(recipe->inputs[slotX].itemName);
}

bool GUIMachine::tryApplyRecipeLayout(const RecipeDefinition& recipe) const {
    if (!m_MachineInventories) {
        return false;
    }

    auto* inventory = m_MachineInventories->get(m_SelectedMachine);
    if (!inventory) {
        return false;
    }

    const int inputSlots = std::max(1, static_cast<int>(recipe.inputs.size()));
    const int outputSlots = static_cast<int>(recipe.outputs.size());

    if (!inventory->inputInventory.resizePreserve(inputSlots, 1)) {
        return false;
    }

    if (!inventory->outputInventory.resizePreserve(outputSlots, outputSlots > 0 ? 1 : 0)) {
        return false;
    }

    return true;
}


const ItemDefinition * GUIMachine::getOutputSlotBackgroundItem(int slotX, int slotY) const {
    if (slotY != 0 || slotX < 0 || !m_ItemDatabase) return nullptr;

    const RecipeDefinition* recipe = getSelectedRecipe();
    if (!recipe) return nullptr;

    if (slotX >= static_cast<int>(recipe->outputs.size())) return nullptr;

    return m_ItemDatabase->getItem(recipe->outputs[slotX].itemName);
}

bool GUIMachine::canAcceptOutputStack(const ItemStack &stack) const {
    if (stack.isEmpty() || !stack.item || !m_CraftingMachines || !m_RecipeDatabase) return false;

    auto* machine = m_CraftingMachines->get(m_SelectedMachine);
    if (!machine) return false;

    const RecipeDefinition* recipe = m_RecipeDatabase->getRecipe(machine->currentRecipeName);
    if (!recipe) return false;

    for (const auto& ingredient : recipe->outputs) {
        if (ingredient.itemName == stack.item->uniqueName)
            return true;
    }

    return false;
}

bool GUIMachine::canAcceptOutputStackAtSlot(int slotX, int slotY, const ItemStack &stack) const {
    if (slotY != 0 || slotX < 0 || stack.isEmpty() || !stack.item) return false;

    const RecipeDefinition* recipe = getSelectedRecipe();
    if (!recipe) return false;

    if (slotX >= static_cast<int>(recipe->outputs.size())) return false;

    return recipe->outputs[slotX].itemName == stack.item->uniqueName;
}
