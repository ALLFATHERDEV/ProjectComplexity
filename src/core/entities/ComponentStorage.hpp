#pragma once
#include <unordered_map>
#include "Entity.hpp"

template<typename T>
class ComponentStorage {
public:
    void add(Entity e, const T& component) {
        if (has(e)) {
            m_Components[m_EntityToIndex[e]] = component;
            return;
        }

        size_t index = m_Components.size();

        m_Components.push_back(component);
        m_Entities.push_back(e);

        m_EntityToIndex[e] = index;
        m_IndexToEntity[index] = e;
    }

    bool has(Entity entity) const {
        return m_EntityToIndex.find(entity) != m_EntityToIndex.end();
    }

    T* get(Entity e) {
        auto it = m_EntityToIndex.find(e);
        if (it == m_EntityToIndex.end())
            return nullptr;

        return &m_Components[it->second];
    }

    const T* get(Entity e) const {
        auto it = m_EntityToIndex.find(e);
        if (it == m_EntityToIndex.end())
            return nullptr;

        return &m_Components[it->second];
    }

    void remove(Entity e) {
        auto it = m_EntityToIndex.find(e);
        if (it == m_EntityToIndex.end())
            return;

        size_t indexToRemove = it->second;
        size_t lastIndex = m_Components.size() - 1;

        Entity lastEntity = m_Entities[lastIndex];

        m_Components[indexToRemove] = m_Components[lastIndex];
        m_Entities[indexToRemove] = lastEntity;

        m_EntityToIndex[lastEntity] = indexToRemove;

        m_Components.pop_back();
        m_Entities.pop_back();

        m_EntityToIndex.erase(e);
    }

    std::vector<T>& getRaw() {
        return m_Components;
    }

    std::vector<Entity>& getEntities() {
        return m_Entities;
    }

private:
    std::vector<T> m_Components;
    std::vector<Entity> m_Entities;

    std::unordered_map<Entity, size_t> m_EntityToIndex;
    std::unordered_map<size_t, Entity> m_IndexToEntity;

};
