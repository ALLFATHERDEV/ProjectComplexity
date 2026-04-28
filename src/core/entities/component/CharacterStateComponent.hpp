#pragma once

enum class Direction {
    DOWN,
    UP,
    LEFT,
    RIGHT
};

enum class CharacterState {
    IDLE,
    WALK
};

struct CharacterStateComponent {
    CharacterState state = CharacterState::IDLE;
    Direction direction = Direction::DOWN;
};