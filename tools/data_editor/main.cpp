#include <SDL3/SDL.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "misc/cpp/imgui_stdlib.h"
#include <nlohmann/json.hpp>

namespace {
using json = nlohmann::json;

enum class EditorSection {
    Items,
    Recipes,
    Entity
};

struct ItemRecord {
    std::filesystem::path filePath;
    bool dirty = false;
    std::string uniqueName;
    std::string displayName;
    int id = -1;
    int maxStackSize = 1;
    int iconAtlasX = 0;
    int iconAtlasY = 0;
    float fuelValue = 0.0f;
    bool isPlaceable = false;
    std::string placeableTexturePath;
    int placeableWidthTiles = 1;
    int placeableHeightTiles = 1;
    bool placeableBlocking = true;
    int placeableLayer = 1;
    std::string placedMachineUniqueName;
    bool placesStorageContainer = false;
    int containerInventoryWidth = 4;
    int containerInventoryHeight = 4;
};

struct RecipeInputRecord {
    std::string itemName;
    int amount = 1;
};

struct RecipeRecord {
    std::filesystem::path filePath;
    bool dirty = false;
    std::string uniqueName;
    std::string displayName;
    std::vector<RecipeInputRecord> inputs;
    std::vector<RecipeInputRecord> outputs;
    float craftTime = 1.0f;
};

struct MachineRecord {
    std::filesystem::path filePath;
    bool dirty = false;
    std::string uniqueName;
    std::string displayName;
    std::string type;
    std::vector<std::string> availableRecipes;
    std::vector<std::string> allowedPlacementTags;
    int spriteAtlasX = 0;
    int spriteAtlasY = 0;
    int widthTiles = 1;
    int heightTiles = 1;
    bool requiresFuel = true;
    int fuelWidth = 1;
    int fuelHeight = 1;
    float miningSpeed = 1.0f;
};

struct EditorDataStore {
    std::filesystem::path projectRoot;
    std::vector<ItemRecord> items;
    std::vector<RecipeRecord> recipes;
    std::vector<MachineRecord> machines;
    std::vector<std::string> loadErrors;
    std::string statusMessage;
    bool statusIsError = false;
    std::array<std::string, 3> searchTerms;
    bool showUnsavedChangesModal = false;
    std::string pendingAction;
    EditorSection pendingSection = EditorSection::Items;
};

void clampSelectedIndex(int& selectedIndex, std::size_t count);

std::string sectionLabel(const EditorSection section) {
    switch (section) {
        case EditorSection::Items:
            return "Items";
        case EditorSection::Recipes:
            return "Rezepte";
        case EditorSection::Entity:
            return "Entity";
        default:
            return "Unknown";
    }
}

std::filesystem::path findProjectRoot() {
    std::filesystem::path current = std::filesystem::current_path();

    while (!current.empty()) {
        if (std::filesystem::exists(current / "assets" / "items") &&
            std::filesystem::exists(current / "assets" / "recipes") &&
            std::filesystem::exists(current / "assets" / "machines") &&
            std::filesystem::exists(current / "CMakeLists.txt")) {
            return current;
        }

        if (current == current.root_path()) {
            break;
        }

        current = current.parent_path();
    }

    return {};
}

template <typename TRecord, typename TLoader>
void loadJsonFolder(const std::filesystem::path& folderPath,
                    std::vector<TRecord>& target,
                    std::vector<std::string>& loadErrors,
                    TLoader loader) {
    target.clear();

    if (!std::filesystem::exists(folderPath)) {
        loadErrors.push_back("Folder not found: " + folderPath.string());
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }

        try {
            std::ifstream file(entry.path());
            if (!file.is_open()) {
                loadErrors.push_back("Could not open file: " + entry.path().string());
                continue;
            }

            json data;
            file >> data;
            target.push_back(loader(entry.path(), data));
        } catch (const std::exception& exception) {
            loadErrors.push_back("Fehler in " + entry.path().string() + ": " + exception.what());
        }
    }
}

EditorDataStore loadEditorData() {
    EditorDataStore dataStore;
    dataStore.projectRoot = findProjectRoot();

    if (dataStore.projectRoot.empty()) {
        dataStore.loadErrors.push_back("Could not find project root.");
        return dataStore;
    }

    loadJsonFolder(
        dataStore.projectRoot / "assets" / "items",
        dataStore.items,
        dataStore.loadErrors,
        [](const std::filesystem::path& filePath, const json& data) {
            ItemRecord item;
            item.filePath = filePath;
            item.uniqueName = data.value("uniqueName", "");
            item.displayName = data.value("displayName", "");
            item.id = data.value("id", -1);
            item.maxStackSize = data.value("maxStackSize", 1);
            item.iconAtlasX = data.value("iconAtlasX", 0);
            item.iconAtlasY = data.value("iconAtlasY", 0);
            item.fuelValue = data.value("fuelValue", 0.0f);
            item.isPlaceable = data.value("isPlaceable", false);
            item.placeableTexturePath = data.value("placeableTexturePath", "");
            item.placeableWidthTiles = data.value("placeableWidthTiles", 1);
            item.placeableHeightTiles = data.value("placeableHeightTiles", 1);
            item.placeableBlocking = data.value("placeableBlocking", true);
            item.placeableLayer = data.value("placeableLayer", 1);
            item.placedMachineUniqueName = data.value("placedMachineUniqueName", "");
            item.placesStorageContainer = data.value("placesStorageContainer", false);
            item.containerInventoryWidth = data.value("containerInventoryWidth", 4);
            item.containerInventoryHeight = data.value("containerInventoryHeight", 4);
            return item;
        }
    );

    loadJsonFolder(
        dataStore.projectRoot / "assets" / "recipes",
        dataStore.recipes,
        dataStore.loadErrors,
        [](const std::filesystem::path& filePath, const json& data) {
            RecipeRecord recipe;
            recipe.filePath = filePath;
            recipe.uniqueName = data.value("uniqueName", "");
            recipe.displayName = data.value("displayName", "");
            recipe.craftTime = data.value("craftTime", 1.0f);

            if (data.contains("inputs") && data["inputs"].is_array()) {
                for (const auto& inputData : data["inputs"]) {
                    RecipeInputRecord input;
                    input.itemName = inputData.value("itemName", inputData.value("item_name", ""));
                    input.amount = inputData.value("amount", 1);
                    recipe.inputs.push_back(std::move(input));
                }
            }

            if (data.contains("outputs") && data["outputs"].is_array()) {
                for (const auto& outputData : data["outputs"]) {
                    RecipeInputRecord output;
                    output.itemName = outputData.value("itemName", outputData.value("item_name", ""));
                    output.amount = outputData.value("amount", 1);
                    recipe.outputs.push_back(std::move(output));
                }
            } else {
                RecipeInputRecord legacyOutput;
                legacyOutput.itemName = data.value("outputItemName", "");
                legacyOutput.amount = data.value("outputAmount", 1);
                if (!legacyOutput.itemName.empty()) {
                    recipe.outputs.push_back(std::move(legacyOutput));
                }
            }

            return recipe;
        }
    );

    loadJsonFolder(
        dataStore.projectRoot / "assets" / "machines",
        dataStore.machines,
        dataStore.loadErrors,
        [](const std::filesystem::path& filePath, const json& data) {
            MachineRecord machine;
            machine.filePath = filePath;
            machine.uniqueName = data.value("uniqueName", "");
            machine.displayName = data.value("displayName", "");
            machine.type = data.value("type", "");
            machine.spriteAtlasX = data.value("spriteAtlasX", 0);
            machine.spriteAtlasY = data.value("spriteAtlasY", 0);
            machine.widthTiles = data.value("widthTiles", 1);
            machine.heightTiles = data.value("heightTiles", 1);
            machine.requiresFuel = data.value("requiresFuel", true);
            machine.fuelWidth = data.value("fuelWidth", 1);
            machine.fuelHeight = data.value("fuelHeight", 1);
            machine.miningSpeed = data.value("miningSpeed", 1.0f);

            if (data.contains("availableRecipes") && data["availableRecipes"].is_array()) {
                for (const auto& recipeName : data["availableRecipes"]) {
                    if (recipeName.is_string()) {
                        machine.availableRecipes.push_back(recipeName.get<std::string>());
                    }
                }
            }

            if (data.contains("allowedPlacementTags") && data["allowedPlacementTags"].is_array()) {
                for (const auto& tag : data["allowedPlacementTags"]) {
                    if (tag.is_string()) {
                        machine.allowedPlacementTags.push_back(tag.get<std::string>());
                    }
                }
            }

            return machine;
        }
    );

    return dataStore;
}

std::filesystem::path sectionFolderPath(const EditorDataStore& dataStore, const EditorSection section) {
    switch (section) {
        case EditorSection::Items:
            return dataStore.projectRoot / "assets" / "items";
        case EditorSection::Recipes:
            return dataStore.projectRoot / "assets" / "recipes";
        case EditorSection::Entity:
            return dataStore.projectRoot / "assets" / "machines";
        default:
            return {};
    }
}

std::string sanitizeFileStem(std::string value, const std::string& fallback) {
    for (char& character : value) {
        const bool valid =
            (character >= 'a' && character <= 'z') ||
            (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') ||
            character == '_' || character == '-';
        if (!valid) {
            character = '_';
        }
    }

    if (value.empty()) {
        return fallback;
    }

    return value;
}

std::filesystem::path buildUniqueFilePath(const std::filesystem::path& folderPath, const std::string& baseName) {
    std::string stem = sanitizeFileStem(baseName, "new_entry");
    std::filesystem::path filePath = folderPath / (stem + ".json");

    int suffix = 1;
    while (std::filesystem::exists(filePath)) {
        filePath = folderPath / (stem + "_" + std::to_string(suffix) + ".json");
        ++suffix;
    }

    return filePath;
}

std::filesystem::path buildUniqueFilePathForRename(const std::filesystem::path& currentFilePath,
                                                   const std::string& baseName) {
    const std::filesystem::path folderPath = currentFilePath.parent_path();
    const std::string stem = sanitizeFileStem(baseName, "new_entry");
    std::filesystem::path filePath = folderPath / (stem + ".json");

    if (filePath == currentFilePath) {
        return filePath;
    }

    int suffix = 1;
    while (std::filesystem::exists(filePath)) {
        filePath = folderPath / (stem + "_" + std::to_string(suffix) + ".json");
        if (filePath == currentFilePath) {
            return filePath;
        }
        ++suffix;
    }

    return filePath;
}

bool hasDuplicateItemUniqueName(const EditorDataStore& dataStore, const ItemRecord& item) {
    int matches = 0;
    for (const ItemRecord& other : dataStore.items) {
        if (other.uniqueName == item.uniqueName) {
            ++matches;
        }
    }
    return matches > 1;
}

bool hasDuplicateRecipeUniqueName(const EditorDataStore& dataStore, const RecipeRecord& recipe) {
    int matches = 0;
    for (const RecipeRecord& other : dataStore.recipes) {
        if (other.uniqueName == recipe.uniqueName) {
            ++matches;
        }
    }
    return matches > 1;
}

bool hasDuplicateMachineUniqueName(const EditorDataStore& dataStore, const MachineRecord& machine) {
    int matches = 0;
    for (const MachineRecord& other : dataStore.machines) {
        if (other.uniqueName == machine.uniqueName) {
            ++matches;
        }
    }
    return matches > 1;
}

bool validateItemRecord(const EditorDataStore& dataStore, const ItemRecord& item, std::string& errorMessage) {
    if (item.uniqueName.empty()) {
        errorMessage = "Item requires a unique name.";
        return false;
    }
    if (hasDuplicateItemUniqueName(dataStore, item)) {
        errorMessage = "Item Unique Name existiert bereits.";
        return false;
    }
    if (item.displayName.empty()) {
        errorMessage = "Item requires a display name.";
        return false;
    }
    if (item.maxStackSize < 1) {
        errorMessage = "Max Stack Size must be at least 1.";
        return false;
    }
    if (item.isPlaceable) {
        if (item.placeableTexturePath.empty()) {
            errorMessage = "Placeable item requires a texture path.";
            return false;
        }
        if (item.placeableWidthTiles < 1 || item.placeableHeightTiles < 1) {
            errorMessage = "Placeable size must be at least 1x1.";
            return false;
        }
    }
    if (item.placesStorageContainer) {
        if (item.containerInventoryWidth < 1 || item.containerInventoryHeight < 1) {
            errorMessage = "Container size must be at least 1x1.";
            return false;
        }
    }
    return true;
}

bool validateRecipeRecord(const EditorDataStore& dataStore, const RecipeRecord& recipe, std::string& errorMessage) {
    if (recipe.uniqueName.empty()) {
        errorMessage = "Recipe requires a unique name.";
        return false;
    }
    if (hasDuplicateRecipeUniqueName(dataStore, recipe)) {
        errorMessage = "Rezept Unique Name existiert bereits.";
        return false;
    }
    if (recipe.displayName.empty()) {
        errorMessage = "Recipe requires a display name.";
        return false;
    }
    if (recipe.outputs.empty()) {
        errorMessage = "Recipe requires at least one output item.";
        return false;
    }
    for (const RecipeInputRecord& input : recipe.inputs) {
        if (input.itemName.empty()) {
            errorMessage = "Each input requires an item name.";
            return false;
        }
        if (input.amount < 1) {
            errorMessage = "Each input amount must be at least 1.";
            return false;
        }
    }
    for (const RecipeInputRecord& output : recipe.outputs) {
        if (output.itemName.empty()) {
            errorMessage = "Each output requires an item name.";
            return false;
        }
        if (output.amount < 1) {
            errorMessage = "Each output amount must be at least 1.";
            return false;
        }
    }
    return true;
}

bool validateMachineRecord(const EditorDataStore& dataStore, const MachineRecord& machine, std::string& errorMessage) {
    if (machine.uniqueName.empty()) {
        errorMessage = "Entity requires a unique name.";
        return false;
    }
    if (hasDuplicateMachineUniqueName(dataStore, machine)) {
        errorMessage = "Entity Unique Name existiert bereits.";
        return false;
    }
    if (machine.displayName.empty()) {
        errorMessage = "Entity requires a display name.";
        return false;
    }
    if (machine.type != "crafting" && machine.type != "miner") {
        errorMessage = "Type must be 'crafting' or 'miner'.";
        return false;
    }
    if (machine.widthTiles < 1 || machine.heightTiles < 1) {
        errorMessage = "Entity size must be at least 1x1.";
        return false;
    }
    if (machine.fuelWidth < 1 || machine.fuelHeight < 1) {
        errorMessage = "Fuel size must be at least 1x1.";
        return false;
    }
    for (const std::string& recipeName : machine.availableRecipes) {
        if (recipeName.empty()) {
            errorMessage = "Available Recipes must not contain empty entries.";
            return false;
        }
    }
    for (const std::string& tag : machine.allowedPlacementTags) {
        if (tag.empty()) {
            errorMessage = "Allowed Placement Tags must not contain empty entries.";
            return false;
        }
    }
    if (machine.type == "miner" && machine.miningSpeed < 0.0f) {
        errorMessage = "Mining Speed must not be negative.";
        return false;
    }
    return true;
}

template <typename TLabelGetter>
bool renderStringSelection(const char* label,
                           std::string& value,
                           int& selectedIndex,
                           const int optionCount,
                           TLabelGetter labelGetter,
                           const bool allowEmpty = false,
                           const char* emptyLabel = "<None>") {
    bool changed = false;
    const char* preview = emptyLabel;

    if (selectedIndex >= 0 && selectedIndex < optionCount) {
        preview = labelGetter(selectedIndex);
    } else if (!value.empty()) {
        preview = value.c_str();
    }

    if (ImGui::BeginCombo(label, preview)) {
        if (allowEmpty) {
            const bool isSelected = selectedIndex < 0;
            if (ImGui::Selectable(emptyLabel, isSelected)) {
                value.clear();
                selectedIndex = -1;
                changed = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        for (int index = 0; index < optionCount; ++index) {
            const char* optionLabel = labelGetter(index);
            const bool isSelected = selectedIndex == index;
            if (ImGui::Selectable(optionLabel, isSelected)) {
                value = optionLabel;
                selectedIndex = index;
                changed = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    return changed;
}

int findItemIndexByUniqueName(const EditorDataStore& dataStore, const std::string& uniqueName) {
    for (int index = 0; index < static_cast<int>(dataStore.items.size()); ++index) {
        if (dataStore.items[index].uniqueName == uniqueName) {
            return index;
        }
    }
    return -1;
}

int findRecipeIndexByUniqueName(const EditorDataStore& dataStore, const std::string& uniqueName) {
    for (int index = 0; index < static_cast<int>(dataStore.recipes.size()); ++index) {
        if (dataStore.recipes[index].uniqueName == uniqueName) {
            return index;
        }
    }
    return -1;
}

int findMachineIndexByUniqueName(const EditorDataStore& dataStore, const std::string& uniqueName) {
    for (int index = 0; index < static_cast<int>(dataStore.machines.size()); ++index) {
        if (dataStore.machines[index].uniqueName == uniqueName) {
            return index;
        }
    }
    return -1;
}

std::string toLowerCopy(std::string value) {
    for (char& character : value) {
        if (character >= 'A' && character <= 'Z') {
            character = static_cast<char>(character - 'A' + 'a');
        }
    }
    return value;
}

bool matchesSearchTerm(const std::string& value, const std::string& searchTerm) {
    if (searchTerm.empty()) {
        return true;
    }

    const std::string haystack = toLowerCopy(value);
    const std::string needle = toLowerCopy(searchTerm);
    return haystack.find(needle) != std::string::npos;
}

bool saveItemRecord(ItemRecord& item, std::string& errorMessage) {
    const std::filesystem::path targetFilePath = buildUniqueFilePathForRename(item.filePath, item.uniqueName);
    json data;
    data["uniqueName"] = item.uniqueName;
    data["id"] = item.id;
    data["displayName"] = item.displayName;
    data["maxStackSize"] = item.maxStackSize;
    data["iconAtlasX"] = item.iconAtlasX;
    data["iconAtlasY"] = item.iconAtlasY;
    data["fuelValue"] = item.fuelValue;
    data["isPlaceable"] = item.isPlaceable;

    if (item.isPlaceable) {
        data["placeableTexturePath"] = item.placeableTexturePath;
        data["placeableWidthTiles"] = item.placeableWidthTiles;
        data["placeableHeightTiles"] = item.placeableHeightTiles;
        data["placeableBlocking"] = item.placeableBlocking;
        data["placeableLayer"] = item.placeableLayer;

        if (!item.placedMachineUniqueName.empty()) {
            data["placedMachineUniqueName"] = item.placedMachineUniqueName;
        }
    }

    if (item.placesStorageContainer) {
        data["placesStorageContainer"] = true;
        data["containerInventoryWidth"] = item.containerInventoryWidth;
        data["containerInventoryHeight"] = item.containerInventoryHeight;
    }

    try {
        std::ofstream file(targetFilePath);
        if (!file.is_open()) {
            errorMessage = "Could not open file for writing: " + targetFilePath.string();
            return false;
        }

        file << data.dump(2) << '\n';
        file.close();

        if (targetFilePath != item.filePath && std::filesystem::exists(item.filePath)) {
            std::filesystem::remove(item.filePath);
        }

        item.filePath = targetFilePath;
        return true;
    } catch (const std::exception& exception) {
        errorMessage = exception.what();
        return false;
    }
}

bool saveRecipeRecord(RecipeRecord& recipe, std::string& errorMessage) {
    const std::filesystem::path targetFilePath = buildUniqueFilePathForRename(recipe.filePath, recipe.uniqueName);
    json data;
    data["uniqueName"] = recipe.uniqueName;
    data["displayName"] = recipe.displayName;
    data["craftTime"] = recipe.craftTime;

    data["inputs"] = json::array();
    for (const RecipeInputRecord& input : recipe.inputs) {
        data["inputs"].push_back({
            {"itemName", input.itemName},
            {"amount", input.amount}
        });
    }

    data["outputs"] = json::array();
    for (const RecipeInputRecord& output : recipe.outputs) {
        data["outputs"].push_back({
            {"itemName", output.itemName},
            {"amount", output.amount}
        });
    }

    try {
        std::ofstream file(targetFilePath);
        if (!file.is_open()) {
            errorMessage = "Could not open file for writing: " + targetFilePath.string();
            return false;
        }

        file << data.dump(2) << '\n';
        file.close();

        if (targetFilePath != recipe.filePath && std::filesystem::exists(recipe.filePath)) {
            std::filesystem::remove(recipe.filePath);
        }

        recipe.filePath = targetFilePath;
        return true;
    } catch (const std::exception& exception) {
        errorMessage = exception.what();
        return false;
    }
}

bool saveMachineRecord(MachineRecord& machine, std::string& errorMessage) {
    const std::filesystem::path targetFilePath = buildUniqueFilePathForRename(machine.filePath, machine.uniqueName);
    json data;
    data["uniqueName"] = machine.uniqueName;
    data["displayName"] = machine.displayName;
    data["type"] = machine.type;
    data["spriteAtlasX"] = machine.spriteAtlasX;
    data["spriteAtlasY"] = machine.spriteAtlasY;
    data["widthTiles"] = machine.widthTiles;
    data["heightTiles"] = machine.heightTiles;
    data["requiresFuel"] = machine.requiresFuel;
    data["fuelWidth"] = machine.fuelWidth;
    data["fuelHeight"] = machine.fuelHeight;

    data["availableRecipes"] = json::array();
    for (const std::string& recipeName : machine.availableRecipes) {
        data["availableRecipes"].push_back(recipeName);
    }

    data["allowedPlacementTags"] = json::array();
    for (const std::string& tag : machine.allowedPlacementTags) {
        data["allowedPlacementTags"].push_back(tag);
    }

    if (machine.type == "miner") {
        data["miningSpeed"] = machine.miningSpeed;
    }

    try {
        std::ofstream file(targetFilePath);
        if (!file.is_open()) {
            errorMessage = "Could not open file for writing: " + targetFilePath.string();
            return false;
        }

        file << data.dump(2) << '\n';
        file.close();

        if (targetFilePath != machine.filePath && std::filesystem::exists(machine.filePath)) {
            std::filesystem::remove(machine.filePath);
        }

        machine.filePath = targetFilePath;
        return true;
    } catch (const std::exception& exception) {
        errorMessage = exception.what();
        return false;
    }
}

template <typename TRecord>
const char* dirtyMarker(const TRecord& record) {
    return record.dirty ? "*" : "";
}

bool saveAllDirtyRecords(EditorDataStore& dataStore) {
    int savedCount = 0;

    for (ItemRecord& item : dataStore.items) {
        if (!item.dirty) {
            continue;
        }

        std::string errorMessage;
        if (!validateItemRecord(dataStore, item, errorMessage)) {
            dataStore.statusMessage = "Save All failed for item '" + item.uniqueName + "': " + errorMessage;
            dataStore.statusIsError = true;
            return false;
        }

        if (!saveItemRecord(item, errorMessage)) {
            dataStore.statusMessage = "Save All failed for item '" + item.uniqueName + "': " + errorMessage;
            dataStore.statusIsError = true;
            return false;
        }

        item.dirty = false;
        ++savedCount;
    }

    for (RecipeRecord& recipe : dataStore.recipes) {
        if (!recipe.dirty) {
            continue;
        }

        std::string errorMessage;
        if (!validateRecipeRecord(dataStore, recipe, errorMessage)) {
            dataStore.statusMessage = "Save All failed for recipe '" + recipe.uniqueName + "': " + errorMessage;
            dataStore.statusIsError = true;
            return false;
        }

        if (!saveRecipeRecord(recipe, errorMessage)) {
            dataStore.statusMessage = "Save All failed for recipe '" + recipe.uniqueName + "': " + errorMessage;
            dataStore.statusIsError = true;
            return false;
        }

        recipe.dirty = false;
        ++savedCount;
    }

    for (MachineRecord& machine : dataStore.machines) {
        if (!machine.dirty) {
            continue;
        }

        std::string errorMessage;
        if (!validateMachineRecord(dataStore, machine, errorMessage)) {
            dataStore.statusMessage = "Save All failed for entity '" + machine.uniqueName + "': " + errorMessage;
            dataStore.statusIsError = true;
            return false;
        }

        if (!saveMachineRecord(machine, errorMessage)) {
            dataStore.statusMessage = "Save All failed for entity '" + machine.uniqueName + "': " + errorMessage;
            dataStore.statusIsError = true;
            return false;
        }

        machine.dirty = false;
        ++savedCount;
    }

    dataStore.statusMessage = "Save All completed. Saved: " + std::to_string(savedCount);
    dataStore.statusIsError = false;
    return true;
}

bool hasAnyDirtyRecords(const EditorDataStore& dataStore) {
    for (const ItemRecord& item : dataStore.items) {
        if (item.dirty) {
            return true;
        }
    }
    for (const RecipeRecord& recipe : dataStore.recipes) {
        if (recipe.dirty) {
            return true;
        }
    }
    for (const MachineRecord& machine : dataStore.machines) {
        if (machine.dirty) {
            return true;
        }
    }
    return false;
}

void requestPendingAction(EditorDataStore& dataStore, const std::string& action, const EditorSection section) {
    dataStore.pendingAction = action;
    dataStore.pendingSection = section;
    dataStore.showUnsavedChangesModal = true;
    ImGui::OpenPopup("UnsavedChanges");
}

void performReload(EditorDataStore& dataStore, std::array<int, 3>& selectedIndices) {
    dataStore = loadEditorData();
    clampSelectedIndex(selectedIndices[0], dataStore.items.size());
    clampSelectedIndex(selectedIndices[1], dataStore.recipes.size());
    clampSelectedIndex(selectedIndices[2], dataStore.machines.size());
    dataStore.statusMessage = "Data reloaded.";
    dataStore.statusIsError = false;
}

void clampSelectedIndex(int& selectedIndex, const std::size_t count) {
    if (count == 0) {
        selectedIndex = -1;
        return;
    }

    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(count)) {
        selectedIndex = 0;
    }
}

void createNewEntry(EditorDataStore& dataStore, const EditorSection section, std::array<int, 3>& selectedIndices) {
    const std::filesystem::path folderPath = sectionFolderPath(dataStore, section);
    if (folderPath.empty()) {
        dataStore.statusMessage = "Could not determine folder for new entry.";
        dataStore.statusIsError = true;
        return;
    }

    if (section == EditorSection::Items) {
        ItemRecord item;
        item.dirty = true;
        item.uniqueName = "new_item";
        item.displayName = "New Item";
        item.filePath = buildUniqueFilePath(folderPath, item.uniqueName);
        dataStore.items.push_back(item);
        selectedIndices[0] = static_cast<int>(dataStore.items.size()) - 1;
    } else if (section == EditorSection::Recipes) {
        RecipeRecord recipe;
        recipe.dirty = true;
        recipe.uniqueName = "new_recipe";
        recipe.displayName = "New Recipe";
        recipe.outputs.push_back({"item_name", 1});
        recipe.filePath = buildUniqueFilePath(folderPath, recipe.uniqueName);
        dataStore.recipes.push_back(recipe);
        selectedIndices[1] = static_cast<int>(dataStore.recipes.size()) - 1;
    } else {
        MachineRecord machine;
        machine.dirty = true;
        machine.uniqueName = "new_machine";
        machine.displayName = "New Machine";
        machine.type = "crafting";
        machine.filePath = buildUniqueFilePath(folderPath, machine.uniqueName);
        dataStore.machines.push_back(machine);
        selectedIndices[2] = static_cast<int>(dataStore.machines.size()) - 1;
    }

    dataStore.statusMessage = sectionLabel(section) + " entry created. Edit and save it now.";
    dataStore.statusIsError = false;
}

void deleteCurrentEntry(EditorDataStore& dataStore, const EditorSection section, std::array<int, 3>& selectedIndices) {
    try {
        if (section == EditorSection::Items) {
            const int index = selectedIndices[0];
            if (index < 0 || index >= static_cast<int>(dataStore.items.size())) {
                return;
            }
            const std::filesystem::path filePath = dataStore.items[index].filePath;
            if (std::filesystem::exists(filePath)) {
                std::filesystem::remove(filePath);
            }
            dataStore.items.erase(dataStore.items.begin() + index);
            clampSelectedIndex(selectedIndices[0], dataStore.items.size());
        } else if (section == EditorSection::Recipes) {
            const int index = selectedIndices[1];
            if (index < 0 || index >= static_cast<int>(dataStore.recipes.size())) {
                return;
            }
            const std::filesystem::path filePath = dataStore.recipes[index].filePath;
            if (std::filesystem::exists(filePath)) {
                std::filesystem::remove(filePath);
            }
            dataStore.recipes.erase(dataStore.recipes.begin() + index);
            clampSelectedIndex(selectedIndices[1], dataStore.recipes.size());
        } else {
            const int index = selectedIndices[2];
            if (index < 0 || index >= static_cast<int>(dataStore.machines.size())) {
                return;
            }
            const std::filesystem::path filePath = dataStore.machines[index].filePath;
            if (std::filesystem::exists(filePath)) {
                std::filesystem::remove(filePath);
            }
            dataStore.machines.erase(dataStore.machines.begin() + index);
            clampSelectedIndex(selectedIndices[2], dataStore.machines.size());
        }

        dataStore.statusMessage = sectionLabel(section) + " entry deleted.";
        dataStore.statusIsError = false;
    } catch (const std::exception& exception) {
        dataStore.statusMessage = "Delete failed: " + std::string(exception.what());
        dataStore.statusIsError = true;
    }
}

void renderMainMenuBar(EditorSection& selectedSection, EditorDataStore& dataStore, std::array<int, 3>& selectedIndices) {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }

    if (ImGui::Selectable("Items", selectedSection == EditorSection::Items, 0, ImVec2(80.0f, 0.0f))) {
        selectedSection = EditorSection::Items;
    }

    ImGui::SameLine();
    if (ImGui::Selectable("Rezepte", selectedSection == EditorSection::Recipes, 0, ImVec2(80.0f, 0.0f))) {
        selectedSection = EditorSection::Recipes;
    }

    ImGui::SameLine();
    if (ImGui::Selectable("Entity", selectedSection == EditorSection::Entity, 0, ImVec2(80.0f, 0.0f))) {
        selectedSection = EditorSection::Entity;
    }

    ImGui::SameLine();
    if (ImGui::Button("Save All")) {
        saveAllDirtyRecords(dataStore);
    }

    ImGui::SameLine();
    if (ImGui::Button("Reload")) {
        if (hasAnyDirtyRecords(dataStore)) {
            requestPendingAction(dataStore, "reload", selectedSection);
        } else {
            performReload(dataStore, selectedIndices);
        }
    }

    ImGui::EndMainMenuBar();
}

void renderItemDetails(ItemRecord& item, EditorDataStore& dataStore) {
    bool changed = false;
    ImGui::Text("File: %s", item.filePath.filename().string().c_str());
    ImGui::Text("Path: %s", item.filePath.string().c_str());
    ImGui::Separator();

    changed |= ImGui::InputText("Unique Name", &item.uniqueName);
    changed |= ImGui::InputText("Display Name", &item.displayName);
    changed |= ImGui::InputInt("ID", &item.id);
    changed |= ImGui::InputInt("Max Stack Size", &item.maxStackSize);
    changed |= ImGui::InputInt("Icon Atlas X", &item.iconAtlasX);
    changed |= ImGui::InputInt("Icon Atlas Y", &item.iconAtlasY);
    changed |= ImGui::InputFloat("Fuel Value", &item.fuelValue, 0.1f, 1.0f, "%.2f");
    changed |= ImGui::Checkbox("Is Placeable", &item.isPlaceable);
    changed |= ImGui::Checkbox("Places Storage Container", &item.placesStorageContainer);

    if (item.isPlaceable) {
        ImGui::Separator();
        changed |= ImGui::InputText("Texture Path", &item.placeableTexturePath);
        changed |= ImGui::InputInt("Width Tiles", &item.placeableWidthTiles);
        changed |= ImGui::InputInt("Height Tiles", &item.placeableHeightTiles);
        changed |= ImGui::Checkbox("Blocking", &item.placeableBlocking);
        changed |= ImGui::InputInt("Layer", &item.placeableLayer);

        int placedMachineIndex = findMachineIndexByUniqueName(dataStore, item.placedMachineUniqueName);
        changed |= renderStringSelection(
            "Placed Machine",
            item.placedMachineUniqueName,
            placedMachineIndex,
            static_cast<int>(dataStore.machines.size()),
            [&dataStore](const int index) { return dataStore.machines[index].uniqueName.c_str(); },
            true
        );
    }

    if (item.placesStorageContainer) {
        ImGui::Separator();
        changed |= ImGui::InputInt("Container Width", &item.containerInventoryWidth);
        changed |= ImGui::InputInt("Container Height", &item.containerInventoryHeight);
    }

    if (item.maxStackSize < 1) {
        item.maxStackSize = 1;
        changed = true;
    }
    if (item.placeableWidthTiles < 1) {
        item.placeableWidthTiles = 1;
        changed = true;
    }
    if (item.placeableHeightTiles < 1) {
        item.placeableHeightTiles = 1;
        changed = true;
    }
    if (item.containerInventoryWidth < 1) {
        item.containerInventoryWidth = 1;
        changed = true;
    }
    if (item.containerInventoryHeight < 1) {
        item.containerInventoryHeight = 1;
        changed = true;
    }

    item.dirty |= changed;

    ImGui::Separator();
    if (ImGui::Button("Save Item")) {
        std::string errorMessage;
        if (!validateItemRecord(dataStore, item, errorMessage)) {
            dataStore.statusMessage = "Save failed: " + errorMessage;
            dataStore.statusIsError = true;
        } else if (saveItemRecord(item, errorMessage)) {
            item.dirty = false;
            dataStore.statusMessage = "Item saved: " + item.filePath.filename().string();
            dataStore.statusIsError = false;
        } else {
            dataStore.statusMessage = "Save failed: " + errorMessage;
            dataStore.statusIsError = true;
        }
    }
}

void renderRecipeDetails(RecipeRecord& recipe, EditorDataStore& dataStore) {
    bool changed = false;
    ImGui::Text("File: %s", recipe.filePath.filename().string().c_str());
    ImGui::Text("Path: %s", recipe.filePath.string().c_str());
    ImGui::Separator();

    changed |= ImGui::InputText("Unique Name", &recipe.uniqueName);
    changed |= ImGui::InputText("Display Name", &recipe.displayName);
    changed |= ImGui::InputFloat("Craft Time", &recipe.craftTime, 0.1f, 1.0f, "%.2f");

    if (recipe.craftTime < 0.0f) {
        recipe.craftTime = 0.0f;
        changed = true;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Inputs");

    for (int index = 0; index < static_cast<int>(recipe.inputs.size()); ++index) {
        RecipeInputRecord& input = recipe.inputs[index];
        ImGui::PushID(index);
        int inputItemIndex = findItemIndexByUniqueName(dataStore, input.itemName);
        changed |= renderStringSelection(
            "Item",
            input.itemName,
            inputItemIndex,
            static_cast<int>(dataStore.items.size()),
            [&dataStore](const int itemIndex) { return dataStore.items[itemIndex].uniqueName.c_str(); }
        );
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        changed |= ImGui::InputInt("Amount", &input.amount);
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            recipe.inputs.erase(recipe.inputs.begin() + index);
            changed = true;
            ImGui::PopID();
            break;
        }
        if (input.amount < 1) {
            input.amount = 1;
            changed = true;
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Add Input")) {
        recipe.inputs.push_back({});
        changed = true;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Outputs");

    for (int index = 0; index < static_cast<int>(recipe.outputs.size()); ++index) {
        RecipeInputRecord& output = recipe.outputs[index];
        ImGui::PushID(index + 1000);
        int outputItemIndex = findItemIndexByUniqueName(dataStore, output.itemName);
        changed |= renderStringSelection(
            "Item",
            output.itemName,
            outputItemIndex,
            static_cast<int>(dataStore.items.size()),
            [&dataStore](const int itemIndex) { return dataStore.items[itemIndex].uniqueName.c_str(); }
        );
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        changed |= ImGui::InputInt("Amount", &output.amount);
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            recipe.outputs.erase(recipe.outputs.begin() + index);
            changed = true;
            ImGui::PopID();
            break;
        }
        if (output.amount < 1) {
            output.amount = 1;
            changed = true;
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Add Output")) {
        recipe.outputs.push_back({});
        changed = true;
    }

    recipe.dirty |= changed;

    ImGui::Separator();
    if (ImGui::Button("Save Recipe")) {
        std::string errorMessage;
        if (!validateRecipeRecord(dataStore, recipe, errorMessage)) {
            dataStore.statusMessage = "Save failed: " + errorMessage;
            dataStore.statusIsError = true;
        } else if (saveRecipeRecord(recipe, errorMessage)) {
            recipe.dirty = false;
            dataStore.statusMessage = "Recipe saved: " + recipe.filePath.filename().string();
            dataStore.statusIsError = false;
        } else {
            dataStore.statusMessage = "Save failed: " + errorMessage;
            dataStore.statusIsError = true;
        }
    }
}

void renderMachineDetails(MachineRecord& machine, EditorDataStore& dataStore) {
    bool changed = false;
    ImGui::Text("File: %s", machine.filePath.filename().string().c_str());
    ImGui::Text("Path: %s", machine.filePath.string().c_str());
    ImGui::Separator();

    changed |= ImGui::InputText("Unique Name", &machine.uniqueName);
    changed |= ImGui::InputText("Display Name", &machine.displayName);
    changed |= ImGui::InputText("Type", &machine.type);
    changed |= ImGui::InputInt("Sprite Atlas X", &machine.spriteAtlasX);
    changed |= ImGui::InputInt("Sprite Atlas Y", &machine.spriteAtlasY);
    changed |= ImGui::InputInt("Width Tiles", &machine.widthTiles);
    changed |= ImGui::InputInt("Height Tiles", &machine.heightTiles);
    changed |= ImGui::Checkbox("Requires Fuel", &machine.requiresFuel);
    changed |= ImGui::InputInt("Fuel Width", &machine.fuelWidth);
    changed |= ImGui::InputInt("Fuel Height", &machine.fuelHeight);

    if (machine.type == "miner") {
        changed |= ImGui::InputFloat("Mining Speed", &machine.miningSpeed, 0.1f, 1.0f, "%.2f");
    }

    if (machine.widthTiles < 1) {
        machine.widthTiles = 1;
        changed = true;
    }
    if (machine.heightTiles < 1) {
        machine.heightTiles = 1;
        changed = true;
    }
    if (machine.fuelWidth < 1) {
        machine.fuelWidth = 1;
        changed = true;
    }
    if (machine.fuelHeight < 1) {
        machine.fuelHeight = 1;
        changed = true;
    }
    if (machine.miningSpeed < 0.0f) {
        machine.miningSpeed = 0.0f;
        changed = true;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Available Recipes");
    for (int index = 0; index < static_cast<int>(machine.availableRecipes.size()); ++index) {
        ImGui::PushID(index);
        int recipeIndex = findRecipeIndexByUniqueName(dataStore, machine.availableRecipes[index]);
        changed |= renderStringSelection(
            "Recipe",
            machine.availableRecipes[index],
            recipeIndex,
            static_cast<int>(dataStore.recipes.size()),
            [&dataStore](const int itemIndex) { return dataStore.recipes[itemIndex].uniqueName.c_str(); }
        );
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            machine.availableRecipes.erase(machine.availableRecipes.begin() + index);
            changed = true;
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Add Recipe")) {
        machine.availableRecipes.emplace_back();
        changed = true;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Allowed Placement Tags");
    for (int index = 0; index < static_cast<int>(machine.allowedPlacementTags.size()); ++index) {
        ImGui::PushID(index + 1000);
        changed |= ImGui::InputText("Tag", &machine.allowedPlacementTags[index]);
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            machine.allowedPlacementTags.erase(machine.allowedPlacementTags.begin() + index);
            changed = true;
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Add Tag")) {
        machine.allowedPlacementTags.emplace_back();
        changed = true;
    }

    machine.dirty |= changed;

    ImGui::Separator();
    if (ImGui::Button("Save Entity")) {
        std::string errorMessage;
        if (!validateMachineRecord(dataStore, machine, errorMessage)) {
            dataStore.statusMessage = "Save failed: " + errorMessage;
            dataStore.statusIsError = true;
        } else if (saveMachineRecord(machine, errorMessage)) {
            machine.dirty = false;
            dataStore.statusMessage = "Entity saved: " + machine.filePath.filename().string();
            dataStore.statusIsError = false;
        } else {
            dataStore.statusMessage = "Save failed: " + errorMessage;
            dataStore.statusIsError = true;
        }
    }
}

void renderEntryList(const EditorSection selectedSection, const EditorDataStore& dataStore, int& selectedIndex) {
    if (ImGui::BeginChild("EntryList", ImVec2(300.0f, 0.0f), ImGuiChildFlags_Borders)) {
        ImGui::Text("%s", sectionLabel(selectedSection).c_str());
        ImGui::Separator();

        const std::string& searchTerm = dataStore.searchTerms[static_cast<std::size_t>(selectedSection)];

        if (selectedSection == EditorSection::Items) {
            for (int index = 0; index < static_cast<int>(dataStore.items.size()); ++index) {
                const ItemRecord& item = dataStore.items[index];
                const std::string label = item.uniqueName.empty() ? item.filePath.stem().string() : item.uniqueName;
                if (!matchesSearchTerm(label, searchTerm)) {
                    continue;
                }
                const std::string displayLabel = label + dirtyMarker(item);
                if (ImGui::Selectable(displayLabel.c_str(), selectedIndex == index)) {
                    selectedIndex = index;
                }
            }
        } else if (selectedSection == EditorSection::Recipes) {
            for (int index = 0; index < static_cast<int>(dataStore.recipes.size()); ++index) {
                const RecipeRecord& recipe = dataStore.recipes[index];
                const std::string label = recipe.uniqueName.empty() ? recipe.filePath.stem().string() : recipe.uniqueName;
                if (!matchesSearchTerm(label, searchTerm)) {
                    continue;
                }
                const std::string displayLabel = label + dirtyMarker(recipe);
                if (ImGui::Selectable(displayLabel.c_str(), selectedIndex == index)) {
                    selectedIndex = index;
                }
            }
        } else {
            for (int index = 0; index < static_cast<int>(dataStore.machines.size()); ++index) {
                const MachineRecord& machine = dataStore.machines[index];
                const std::string label = machine.uniqueName.empty() ? machine.filePath.stem().string() : machine.uniqueName;
                if (!matchesSearchTerm(label, searchTerm)) {
                    continue;
                }
                const std::string displayLabel = label + dirtyMarker(machine);
                if (ImGui::Selectable(displayLabel.c_str(), selectedIndex == index)) {
                    selectedIndex = index;
                }
            }
        }
    }
    ImGui::EndChild();
}

void renderEntryToolbar(EditorDataStore& dataStore, const EditorSection selectedSection, std::array<int, 3>& selectedIndices) {
    if (ImGui::Button("New")) {
        createNewEntry(dataStore, selectedSection, selectedIndices);
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
        if (hasAnyDirtyRecords(dataStore)) {
            requestPendingAction(dataStore, "delete", selectedSection);
        } else {
            deleteCurrentEntry(dataStore, selectedSection, selectedIndices);
        }
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(220.0f);
    ImGui::InputText("Search", &dataStore.searchTerms[static_cast<std::size_t>(selectedSection)]);

    ImGui::Separator();
}

void renderDetailsPanel(const EditorSection selectedSection, EditorDataStore& dataStore, const int selectedIndex) {
    ImGui::SameLine();

    if (ImGui::BeginChild("EntryDetails", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders)) {
        if (selectedSection == EditorSection::Items) {
            if (selectedIndex < 0 || selectedIndex >= static_cast<int>(dataStore.items.size())) {
                ImGui::TextUnformatted("No item selected");
            } else {
                ImGui::TextUnformatted(dataStore.items[selectedIndex].dirty ? "Status: Unsaved changes" : "Status: Saved");
                ImGui::Separator();
                renderItemDetails(dataStore.items[selectedIndex], dataStore);
            }
        } else if (selectedSection == EditorSection::Recipes) {
            if (selectedIndex < 0 || selectedIndex >= static_cast<int>(dataStore.recipes.size())) {
                ImGui::TextUnformatted("No recipe selected");
            } else {
                ImGui::TextUnformatted(dataStore.recipes[selectedIndex].dirty ? "Status: Unsaved changes" : "Status: Saved");
                ImGui::Separator();
                renderRecipeDetails(dataStore.recipes[selectedIndex], dataStore);
            }
        } else {
            if (selectedIndex < 0 || selectedIndex >= static_cast<int>(dataStore.machines.size())) {
                ImGui::TextUnformatted("No entity selected");
            } else {
                ImGui::TextUnformatted(dataStore.machines[selectedIndex].dirty ? "Status: Unsaved changes" : "Status: Saved");
                ImGui::Separator();
                renderMachineDetails(dataStore.machines[selectedIndex], dataStore);
            }
        }
    }
    ImGui::EndChild();
}

void renderErrorPanel(const EditorDataStore& dataStore) {
    if (dataStore.loadErrors.empty()) {
        return;
    }

    if (ImGui::Begin("LoadErrors")) {
        for (const std::string& error : dataStore.loadErrors) {
            ImGui::BulletText("%s", error.c_str());
        }
    }
    ImGui::End();
}

void renderStatusBar(const EditorDataStore& dataStore) {
    if (dataStore.statusMessage.empty()) {
        return;
    }

    const ImVec4 color = dataStore.statusIsError
        ? ImVec4(0.90f, 0.30f, 0.30f, 1.00f)
        : ImVec4(0.30f, 0.85f, 0.40f, 1.00f);

    ImGui::SetNextWindowBgAlpha(0.9f);
    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f, viewport->WorkPos.y + 48.0f),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f)
    );

    if (ImGui::Begin("StatusMessage", nullptr, windowFlags)) {
        ImGui::TextColored(color, "%s", dataStore.statusMessage.c_str());
    }
    ImGui::End();
}

void renderUnsavedChangesModal(EditorDataStore& dataStore,
                               bool& running,
                               std::array<int, 3>& selectedIndices) {
    if (dataStore.showUnsavedChangesModal) {
        ImGui::OpenPopup("UnsavedChanges");
    }

    if (!ImGui::BeginPopupModal("UnsavedChanges", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    ImGui::TextUnformatted("There are unsaved changes.");
    ImGui::TextUnformatted("Do you want to save them first?");
    ImGui::Separator();

    if (ImGui::Button("Save All", ImVec2(120.0f, 0.0f))) {
        if (saveAllDirtyRecords(dataStore)) {
            if (dataStore.pendingAction == "reload") {
                performReload(dataStore, selectedIndices);
            } else if (dataStore.pendingAction == "delete") {
                deleteCurrentEntry(dataStore, dataStore.pendingSection, selectedIndices);
            } else if (dataStore.pendingAction == "quit") {
                running = false;
            }
            dataStore.pendingAction.clear();
            dataStore.showUnsavedChangesModal = false;
            ImGui::CloseCurrentPopup();
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Discard", ImVec2(120.0f, 0.0f))) {
        if (dataStore.pendingAction == "reload") {
            performReload(dataStore, selectedIndices);
        } else if (dataStore.pendingAction == "delete") {
            deleteCurrentEntry(dataStore, dataStore.pendingSection, selectedIndices);
        } else if (dataStore.pendingAction == "quit") {
            running = false;
        }
        dataStore.pendingAction.clear();
        dataStore.showUnsavedChangesModal = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f))) {
        dataStore.pendingAction.clear();
        dataStore.showUnsavedChangesModal = false;
        dataStore.statusMessage = "Action canceled.";
        dataStore.statusIsError = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void renderEditorView(const EditorSection selectedSection,
                      EditorDataStore& dataStore,
                      std::array<int, 3>& selectedIndices,
                      bool& running) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float menuBarHeight = ImGui::GetFrameHeight();
    int& selectedIndex = selectedIndices[static_cast<std::size_t>(selectedSection)];

    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + menuBarHeight),
        ImGuiCond_Always
    );
    ImGui::SetNextWindowSize(
        ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - menuBarHeight),
        ImGuiCond_Always
    );

    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar;

    if (ImGui::Begin("EditorRoot", nullptr, windowFlags)) {
        renderEntryToolbar(dataStore, selectedSection, selectedIndices);
        renderEntryList(selectedSection, dataStore, selectedIndex);
        renderDetailsPanel(selectedSection, dataStore, selectedIndex);
    }
    ImGui::End();

    renderErrorPanel(dataStore);
    renderStatusBar(dataStore);
    renderUnsavedChangesModal(dataStore, running, selectedIndices);
}
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "CozyGame Data Editor",
        1280,
        720,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    EditorSection selectedSection = EditorSection::Items;
    EditorDataStore dataStore = loadEditorData();
    std::array<int, 3> selectedIndices{0, 0, 0};
    clampSelectedIndex(selectedIndices[0], dataStore.items.size());
    clampSelectedIndex(selectedIndices[1], dataStore.recipes.size());
    clampSelectedIndex(selectedIndices[2], dataStore.machines.size());
    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT) {
                if (hasAnyDirtyRecords(dataStore)) {
                    requestPendingAction(dataStore, "quit", selectedSection);
                } else {
                    running = false;
                }
            }
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        renderMainMenuBar(selectedSection, dataStore, selectedIndices);
        renderEditorView(selectedSection, dataStore, selectedIndices, running);

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
