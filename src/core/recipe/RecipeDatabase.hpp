#pragma once
#include "RecipeDefinition.hpp"

#include <string>
#include <unordered_map>

class RecipeDatabase {
public:
    void addRecipe(const RecipeDefinition& recipe) {
        m_Recipes[recipe.uniqueName] = recipe;
    }

    const RecipeDefinition* getRecipe(const std::string& uniqueName) const {
        auto it = m_Recipes.find(uniqueName);
        if (it == m_Recipes.end())
            return nullptr;
        return &it->second;
    }

private:
    std::unordered_map<std::string, RecipeDefinition> m_Recipes;
};
