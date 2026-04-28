#ifndef PROJECTCOMPLEXITY_ENTITYMANAGER_H
#define PROJECTCOMPLEXITY_ENTITYMANAGER_H

#include "Entity.hpp"

class EntityManager {
public:
    Entity createEntity() {
        Entity e = m_NextID++;
        m_Entities.push_back(e);
        return e;
    }


private:
    Entity m_NextID = 1;
    std::vector<Entity> m_Entities;
};

#endif //PROJECTCOMPLEXITY_ENTITYMANAGER_H