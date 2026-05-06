#include "RecipeDatabase.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "../Logger.hpp"

using json = nlohmann::json;

bool RecipeDatabase::loadRecipesFromFolder(const std::string& folderPath) {
    m_Recipes.clear();

    if (!std::filesystem::exists(folderPath)) {
        LOG_ERROR("Recipe folder does not exist: {}", folderPath);
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (entry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            LOG_WARN("Could not open recipe file {}", entry.path().string());
            continue;
        }

        json data;
        file >> data;

        RecipeDefinition recipe;
        recipe.uniqueName = data.value("uniqueName", "");
        recipe.displayName = data.value("displayName", "");
        recipe.craftTime = data.value("craftTime", 1.0f);

        //Loading Items
        if (data.contains("itemInputs") && data["itemInputs"].is_array()) {
            for (const auto& input : data["itemInputs"]) {
                RecipeIngredient ingredient;
                ingredient.itemName = input.value("itemName", input.value("item_name", ""));
                ingredient.amount = input.value("amount", 1);
                recipe.itemInputs.push_back(ingredient);
            }
        } else {
            if (data.contains("inputs") && data["inputs"].is_array()) {
                for (const auto& inputData : data["inputs"]) {
                RecipeIngredient ingredient;
                ingredient.itemName = inputData.value("itemName", inputData.value("item_name", ""));
                ingredient.amount = inputData.value("amount", 1);
                recipe.itemInputs.push_back(ingredient);
            }
        }
        }

        if (data.contains("itemOutputs") && data["itemOutputs"].is_array()) {
            for (const auto& outputData : data["itemOutputs"]) {
                RecipeIngredient output;
                output.itemName = outputData.value("itemName", outputData.value("item_name", ""));
                output.amount = outputData.value("amount", 1);
                if (!output.itemName.empty()) {
                    recipe.itemOutputs.push_back(output);
                }
            }
        } else {
            const std::string legacyOutputItemName = data.value("outputItemName", "");
            if (!legacyOutputItemName.empty()) {
                RecipeIngredient output;
                output.itemName = legacyOutputItemName;
                output.amount = data.value("outputAmount", 1);
                recipe.itemOutputs.push_back(output);
            }
        }

        //Loading fluids
        if (data.contains("fluidInputs") && data["fluidInputs"].is_array()) {
            for (const auto& inputData : data["fluidInputs"]) {
                RecipeFluidStack fluidStack;
                fluidStack.slotName = inputData.value("slotName", inputData.value("slot_name", ""));
                fluidStack.fluidName = inputData.value("fluidName", inputData.value("fluid_name", ""));
                fluidStack.amount = inputData.value("amount", 1.0f);
                recipe.fluidInputs.push_back(fluidStack);
            }
        } else if (data.contains("inputFluids") && data["inputFluids"].is_array()) {
            for (const auto& inputData : data["inputFluids"]) {
                RecipeFluidStack fluidStack;
                fluidStack.slotName = inputData.value("slotName", inputData.value("slot_name", ""));
                fluidStack.fluidName = inputData.value("fluidName", inputData.value("fluid_name", ""));
                fluidStack.amount = inputData.value("amount", 1.0f);
                recipe.fluidInputs.push_back(fluidStack);
            }
        }

        if (data.contains("fluidOutputs") && data["fluidOutputs"].is_array()) {
            for (const auto& outputData : data["fluidOutputs"]) {
                RecipeFluidStack fluidStack;
                fluidStack.slotName = outputData.value("slotName", outputData.value("slot_name", ""));
                fluidStack.fluidName = outputData.value("fluidName", outputData.value("fluid_name", ""));
                fluidStack.amount = outputData.value("amount", 1.0f);
                recipe.fluidOutputs.push_back(fluidStack);
            }
        }

        if (recipe.uniqueName.empty()) {
            LOG_WARN("Recipe has no unique name: {}, skipping recipe", entry.path().string());
            continue;
        }

        if (recipe.itemOutputs.empty() && recipe.fluidOutputs.empty()) {
            LOG_WARN("Recipe {} has no outputs, skipping recipe", recipe.uniqueName);
            continue;
        }

        if (m_Recipes.contains(recipe.uniqueName)) {
            LOG_WARN("Recipe with unique name {} already exists, skipping recipe", recipe.uniqueName);
            continue;
        }

        m_Recipes[recipe.uniqueName] = recipe;
    }

    LOG_INFO("Loaded {} recipes from folder {}", m_Recipes.size(), folderPath);
    return true;
}

void RecipeDatabase::addRecipe(const RecipeDefinition& recipe) {
    m_Recipes[recipe.uniqueName] = recipe;
}

std::vector<std::string> RecipeDatabase::getRecipeNames() const {
    std::vector<std::string> recipeNames;
    recipeNames.reserve(m_Recipes.size());

    for (const auto& [recipeName, recipe] : m_Recipes) {
        recipeNames.push_back(recipeName);
    }

    return recipeNames;
}
