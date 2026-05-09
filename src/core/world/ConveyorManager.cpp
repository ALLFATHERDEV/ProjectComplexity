#include "ConveyorManager.hpp"

#include "EntityFactory.hpp"

bool ConveyorManager::pointsTo(Direction direction, int fromX, int fromY, int targetX, int targetY) {
    switch (direction) {
        case Direction::RIGHT:
            return fromX + 1 == targetX && fromY == targetY;
        case Direction::LEFT:
            return fromX - 1 == targetX && fromY == targetY;
        case Direction::DOWN:
            return fromX == targetX && fromY + 1 == targetY;
        case Direction::UP:
            return fromX == targetX && fromY - 1 == targetY;
        default:
            return false;
    }
}

SpriteCoords ConveyorManager::getStraightConveyorSprite(Direction direction) {
    switch (direction) {
        case Direction::RIGHT:
            return {0, 5};
        case Direction::DOWN:
            return {0, 6};
        case Direction::LEFT:
            return {7, 5};
        case Direction::UP:
            return {2, 6};
        default:
            return {0, 5};
    }
}

SpriteCoords ConveyorManager::getCurveConveyorSprite(Direction incomingSide, Direction outgoingDirection) {
    if (incomingSide == Direction::LEFT && outgoingDirection == Direction::DOWN) {
        return {3, 12};
    }

    if (incomingSide == Direction::LEFT && outgoingDirection == Direction::UP) {
        return {0, 11};
    }

    if (incomingSide == Direction::RIGHT && outgoingDirection == Direction::DOWN) {
        return {0, 12};
    }

    if (incomingSide == Direction::RIGHT && outgoingDirection == Direction::UP) {
        return {0, 10};
    }

    if (incomingSide == Direction::UP && outgoingDirection == Direction::LEFT) {
        return {8, 10};
    }

    if (incomingSide == Direction::UP && outgoingDirection == Direction::RIGHT) {
        return {4, 10};
    }

    if (incomingSide == Direction::DOWN && outgoingDirection == Direction::RIGHT) {
        return {4, 11};
    }

    if (incomingSide == Direction::DOWN && outgoingDirection == Direction::LEFT) {
        return {6, 12};
    }

    return getStraightConveyorSprite(outgoingDirection);
}

ConveyorManager::ConveyorManager(EntityManager& entityManager,
                                 ComponentRegistry& components,
                                 AnimationLibrary& animationLibrary)
    : m_EntityManager(entityManager),
      m_Components(components),
      m_AnimationLibrary(animationLibrary) {
}

void ConveyorManager::setAtlas(SpriteAtlas* atlas) {
    m_ConveyorAtlas = atlas;
}

long long ConveyorManager::makeTileKey(int tileX, int tileY) {
    const unsigned long long x = static_cast<unsigned int>(tileX);
    const unsigned long long y = static_cast<unsigned int>(tileY);
    return static_cast<long long>((x << 32) | y);
}

Entity ConveyorManager::getConveyorBeltAt(int tileX, int tileY) const {
    auto it = m_ConveyorEntitiesByTile.find(makeTileKey(tileX, tileY));
    if (it == m_ConveyorEntitiesByTile.end()) {
        return 0;
    }

    return it->second;
}

void ConveyorManager::refreshConveyorSpriteAt(int tileX, int tileY) {
    if (!m_ConveyorAtlas) {
        return;
    }

    const Entity beltEntity = getConveyorBeltAt(tileX, tileY);
    if (beltEntity == 0) {
        return;
    }

    auto* belt = m_Components.m_ConveyorBelts.get(beltEntity);
    auto* sprite = m_Components.m_Sprites.get(beltEntity);
    auto* animationController = m_Components.m_AnimationControllers.get(beltEntity);
    if (!belt) {
        return;
    }

    std::vector<Direction> incomingSides;
    const std::pair<Direction, std::pair<int, int>> neighbors[] = {
        {Direction::UP, {tileX, tileY - 1}},
        {Direction::DOWN, {tileX, tileY + 1}},
        {Direction::LEFT, {tileX - 1, tileY}},
        {Direction::RIGHT, {tileX + 1, tileY}}
    };

    for (const auto& [incomingSide, pos] : neighbors) {
        const Entity neighborEntity = getConveyorBeltAt(pos.first, pos.second);
        if (neighborEntity == 0) {
            continue;
        }

        const auto* neighborBelt = m_Components.m_ConveyorBelts.get(neighborEntity);
        if (!neighborBelt) {
            continue;
        }

        if (pointsTo(neighborBelt->direction, pos.first, pos.second, tileX, tileY)) {
            incomingSides.push_back(incomingSide);
        }
    }

    SpriteCoords coords = getStraightConveyorSprite(belt->direction);
    bool useCurveSprite = false;
    if (incomingSides.size() == 1) {
        const Direction incomingSide = incomingSides.front();
        const bool isStraight =
            (incomingSide == Direction::LEFT && belt->direction == Direction::RIGHT) ||
            (incomingSide == Direction::RIGHT && belt->direction == Direction::LEFT) ||
            (incomingSide == Direction::UP && belt->direction == Direction::DOWN) ||
            (incomingSide == Direction::DOWN && belt->direction == Direction::UP);

        if (!isStraight) {
            coords = getCurveConveyorSprite(incomingSide, belt->direction);
            useCurveSprite = true;
        }
    }

    if (useCurveSprite) {
        if (sprite) {
            sprite->visible = true;
            sprite->sprite = m_ConveyorAtlas->getSprite(coords.x, coords.y);
        }

        if (animationController) {
            animationController->enabled = false;
            animationController->currentAnimation = nullptr;
        }

        return;
    }

    if (sprite) {
        sprite->visible = false;
        sprite->sprite = m_ConveyorAtlas->getSprite(coords.x, coords.y);
    }

    if (animationController) {
        animationController->enabled = true;
        animationController->stateName = "default";
        animationController->useDirection = true;
        animationController->direction = belt->direction;
        animationController->currentAnimation = nullptr;
    }
}

void ConveyorManager::refreshConveyorSpritesAround(int tileX, int tileY) {
    refreshConveyorSpriteAt(tileX, tileY);
    refreshConveyorSpriteAt(tileX + 1, tileY);
    refreshConveyorSpriteAt(tileX - 1, tileY);
    refreshConveyorSpriteAt(tileX, tileY + 1);
    refreshConveyorSpriteAt(tileX, tileY - 1);
}

void ConveyorManager::placeConveyorBelt(int tileX, int tileY, Direction direction) {
    if (!m_ConveyorAtlas) {
        return;
    }

    removeConveyorBelt(tileX, tileY);

    EntityFactory factory(m_EntityManager, m_Components, m_AnimationLibrary);

    const float worldX = static_cast<float>(tileX) * kTileSize;
    const float worldY = static_cast<float>(tileY) * kTileSize;
    Entity belt = factory.createConveyorBelt({ worldX, worldY }, *m_ConveyorAtlas, direction);
    m_ConveyorEntitiesByTile[makeTileKey(tileX, tileY)] = belt;
    refreshConveyorSpritesAround(tileX, tileY);
}

void ConveyorManager::removeConveyorBelt(int tileX, int tileY) {
    const long long key = makeTileKey(tileX, tileY);
    auto it = m_ConveyorEntitiesByTile.find(key);
    if (it == m_ConveyorEntitiesByTile.end()) {
        return;
    }

    const Entity belt = it->second;
    m_Components.m_Positions.remove(belt);
    m_Components.m_Sprites.remove(belt);
    m_Components.m_AnimationControllers.remove(belt);
    m_Components.m_ConveyorBelts.remove(belt);
    m_Components.m_Collisions.remove(belt);
    m_Components.m_Interactions.remove(belt);
    m_ConveyorEntitiesByTile.erase(it);
    refreshConveyorSpritesAround(tileX, tileY);
}

void ConveyorManager::clearConveyorBelts() {
    std::vector<std::pair<int, int>> tilesToClear;
    tilesToClear.reserve(m_ConveyorEntitiesByTile.size());

    for (const auto& [tileKey, entity] : m_ConveyorEntitiesByTile) {
        const int tileX = static_cast<int>(tileKey >> 32);
        const int tileY = static_cast<int>(tileKey & 0xffffffff);
        tilesToClear.emplace_back(tileX, tileY);
    }

    for (const auto& [tileX, tileY] : tilesToClear) {
        removeConveyorBelt(tileX, tileY);
    }
}

std::vector<std::tuple<int, int, Direction>> ConveyorManager::getConveyorBeltData() const {
    std::vector<std::tuple<int, int, Direction>> conveyors;
    conveyors.reserve(m_ConveyorEntitiesByTile.size());

    for (const auto& [tileKey, entity] : m_ConveyorEntitiesByTile) {
        const auto* belt = m_Components.m_ConveyorBelts.get(entity);
        if (!belt) {
            continue;
        }

        const int tileX = static_cast<int>(tileKey >> 32);
        const int tileY = static_cast<int>(tileKey & 0xffffffff);
        conveyors.emplace_back(tileX, tileY, belt->direction);
    }

    return conveyors;
}
