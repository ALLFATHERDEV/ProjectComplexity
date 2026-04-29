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
        recipe.outputItemName = data.value("outputItemName", "");
        recipe.outputAmount = data.value("outputAmount", 1);
        recipe.craftTime = data.value("craftTime", 1.0f);

        if (data.contains("inputs") && data["inputs"].is_array()) {
            for (const auto& inputData : data["inputs"]) {
                RecipeIngredient ingredient;
                ingredient.itemName = inputData.value("itemName", "");
                ingredient.amount = inputData.value("amount", 1);
                recipe.inputs.push_back(ingredient);
            }
        }

        if (recipe.uniqueName.empty()) {
            LOG_WARN("Recipe has no unique name: {}, skipping recipe", entry.path().string());
            continue;
        }

        if (recipe.outputItemName.empty()) {
            LOG_WARN("Recipe {} has no output item, skipping recipe", recipe.uniqueName);
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
