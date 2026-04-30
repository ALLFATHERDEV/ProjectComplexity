#pragma once
#include "../GUISystem.hpp"
#include "../../entities/ComponentStorage.hpp"
#include "../../entities/Entity.hpp"
#include "../../entities/component/CraftingMachineComponent.hpp"
#include "../../entities/component/InventoryComponent.hpp"
#include "../../entities/component/MachineInventoryComponent.hpp"
#include "../../entities/component/MinerComponent.hpp"
#include "../../inventory/ItemDatabase.hpp"
#include "../../recipe/RecipeDatabase.hpp"
#include "../elements/GUIButton.hpp"
#include "../elements/GUIInventoryGrid.hpp"
#include "../elements/GUIPanel.hpp"
#include "../elements/GUIProgressBar.hpp"
#include "../elements/GUIText.hpp"

class GUIMachine {
public:
    void create(GUISystem& guiSystem, GUIDragContext* dragContext);

    void open(Entity machine);
    void openPlayerInventory();
    void togglePlayerInventory();
    void close();
    bool isOpen() const;
    bool isPlayerInventoryOpen() const;

    void bind(ComponentStorage<MachineInventoryComponent>* machineInventories,
              ComponentStorage<CraftingMachineComponent>* craftingMachines,
              ComponentStorage<MinerComponent>* miners,
              const RecipeDatabase* recipeDatabase,
              const ItemDatabase* itemDatabase,
              ComponentStorage<InventoryComponent>* inventories,
              Entity player);

    void update();
    void updateRecipeButtons();

private:
    Entity m_SelectedMachine = 0;
    bool m_Open = false;
    bool m_PlayerInventoryOpen = false;
    GUISystem* m_GUISystem = nullptr;

    const RecipeDatabase* m_RecipeDatabase = nullptr;
    const ItemDatabase* m_ItemDatabase = nullptr;

    ComponentStorage<MachineInventoryComponent>* m_MachineInventories = nullptr;
    ComponentStorage<CraftingMachineComponent>* m_CraftingMachines = nullptr;
    ComponentStorage<MinerComponent>* m_Miners = nullptr;
    ComponentStorage<InventoryComponent>* m_Inventories = nullptr;
    Entity m_Player = 0;

    GUIPanel* m_Panel = nullptr;
    GUIInventoryGrid* m_FuelGrid = nullptr;
    GUIProgressBar* m_FuelProgressBar = nullptr;
    GUIInventoryGrid* m_InputGrid = nullptr;
    GUIInventoryGrid* m_OutputGrid = nullptr;
    GUIProgressBar* m_ProgressBar = nullptr;
    GUIText* m_MinerInfoText = nullptr;
    GUIPanel* m_PlayerInventoryPanel = nullptr;
    GUIInventoryGrid* m_PlayerInventoryGrid = nullptr;
    std::vector<GUIButton*> m_RecipeButtons;

    void showPlayerInventory();
    void hidePlayerInventory();
    bool isCraftingMachine(Entity machine) const;
    bool isMiner(Entity machine) const;
    void rebuildRecipeButtons(GUISystem& guiSystem);
    bool canAcceptInputStack(const ItemStack& stack) const;
    bool tryApplyRecipeLayout(const RecipeDefinition& recipe) const;
};
