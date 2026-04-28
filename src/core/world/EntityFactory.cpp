#include "EntityFactory.hpp"

#include "../entities/component/InventoryComponent.hpp"

Entity EntityFactory::createPlayer(Vec2f position) {
    Entity player = m_EntityManager.createEntity();
    m_Positions.add(player, { position });
    m_Velocities.add(player, { 0.0f, 0.0f});
    m_Inputs.add(player, { 250 });
    m_CharacterStates.add(player, {CharacterState::IDLE, Direction::DOWN} );
    m_Collisions.add(player, { SDL_FRect(4.0f, 8.0f, 24.0f, 24.0f), true, false });

    AnimationControllerComponent controller;
    controller.animations[{ CharacterState::IDLE, Direction::DOWN }] = m_AnimationLibrary.get("player_idle_down");
    controller.animations[{ CharacterState::IDLE, Direction::UP }] = m_AnimationLibrary.get("player_idle_up");
    controller.animations[{ CharacterState::IDLE, Direction::RIGHT }] = m_AnimationLibrary.get("player_idle_right");
    controller.animations[{ CharacterState::IDLE, Direction::LEFT }] = m_AnimationLibrary.get("player_idle_left");

    controller.animations[{ CharacterState::WALK, Direction::LEFT }] = m_AnimationLibrary.get("player_walk_left");
    controller.animations[{ CharacterState::WALK, Direction::RIGHT }] = m_AnimationLibrary.get("player_walk_right");
    controller.animations[{ CharacterState::WALK, Direction::UP }] = m_AnimationLibrary.get("player_walk_up");
    controller.animations[{ CharacterState::WALK, Direction::DOWN }] = m_AnimationLibrary.get("player_walk_down");

    controller.currentAnimation = controller.animations[{ CharacterState::IDLE, Direction::DOWN }];
    controller.currentAnimation->play();
    m_AnimationControllers.add(player, controller);

    InventoryComponent inventory;
    inventory.inventory.create(10, 4);
    m_Inventories.add(player, inventory);

    return player;
}
