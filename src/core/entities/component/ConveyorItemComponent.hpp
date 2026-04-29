#pragma once

#include "../../inventory/ItemDefinition.hpp"
#include "CharacterStateComponent.hpp"

struct ConveyorItemComponent {
    const ItemDefinition* item = nullptr;
    int tileX = 0;
    int tileY = 0;
    Direction direction = Direction::RIGHT;
    float progress = 0.0f;
    float speed = 2.0f;
    bool blocked = false;
};
