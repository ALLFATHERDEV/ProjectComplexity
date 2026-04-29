#pragma once
#include "RecipeDefinition.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class RecipeDatabase {
public:
    bool loadRecipesFromFolder(const std::string& folderPath);
    void addRecipe(const RecipeDefinition& recipe);

    const RecipeDefinition* getRecipe(const std::string& uniqueName) const {
        auto it = m_Recipes.find(uniqueName);
        if (it == m_Recipes.end())
            return nullptr;
        return &it->second;
    }

    const std::unordered_map<std::string, RecipeDefinition>& getAllRecipes() const {
        return m_Recipes;
    }

    std::vector<std::string> getRecipeNames() const;

private:
    std::unordered_map<std::string, RecipeDefinition> m_Recipes;
};
