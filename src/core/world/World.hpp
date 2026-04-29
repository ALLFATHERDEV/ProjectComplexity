#ifndef PROJECTCOMPLEXITY_WORLD_H
#define PROJECTCOMPLEXITY_WORLD_H
#include "AnimationLibrary.hpp"
#include "ChunkManager.hpp"
#include "ConveyorManager.hpp"
#include "../entities/ComponentStorage.hpp"
#include "../entities/EntityManager.hpp"
#include "../entities/component/AnimatedSpriteComponent.hpp"
#include "../entities/component/CollisionComponent.hpp"
#include "../entities/component/ConveyorBeltComponent.hpp"
#include "../entities/component/ConveyorItemComponent.hpp"
#include "../entities/component/CraftingMachineComponent.hpp"
#include "../entities/component/InteractionComponent.hpp"
#include "../entities/component/InventoryComponent.hpp"
#include "../entities/component/MachineInventoryComponent.hpp"
#include "../entities/component/PositionComponent.hpp"
#include "../entities/component/SpriteComponent.hpp"
#include "../entities/systems/AnimatedRenderSystem.hpp"
#include "../entities/systems/AnimationStateSystem.hpp"
#include "../entities/systems/AnimationSystem.hpp"
#include "../entities/systems/CharacterStateSystem.hpp"
#include "../entities/systems/CollisionSystem.hpp"
#include "../entities/systems/ConveyorSystem.hpp"
#include "../entities/systems/CraftingSystem.hpp"
#include "../entities/systems/InputSystem.hpp"
#include "../entities/systems/InteractionSystem.hpp"
#include "../entities/systems/MovementInputSystem.hpp"
#include "../entities/systems/MovementSystem.hpp"
#include "../entities/systems/RenderSystem.hpp"
#include "../graphics/Renderer.hpp"
#include "../graphics/SpriteAtlas.hpp"
#include "../inventory/ItemDatabase.hpp"
#include "../machine/MachineDatabase.hpp"
#include "../map/TileMap.hpp"

struct InputComponent;
struct VelocityComponent;

class World {
public:
    World();
    ~World();

    void setRenderer(Renderer* renderer);
    void initializeWorld();
    void update(float deltaTime);
    void render();
    void handleInput(SDL_Event& event);
    std::optional<Entity> tryInteract(const SDL_Event& event);

    Camera2D& getCamera() { return m_Camera; }
    TileMap& getTileMap() { return m_TileMap; }
    const TileMap& getTileMap() const { return m_TileMap; }
    SpriteAtlas& getTileMapAtlas() { return m_TileMapAtlas; }
    SpriteAtlas& getConveyorAtlas() { return m_ConveyorAtlas; }
    ChunkManager& getChunkManager() { return m_ChunkManager; }
    const ChunkManager& getChunkManager() const { return m_ChunkManager; }
    ComponentStorage<InventoryComponent>& getInventories() { return m_Inventories; }
    ComponentStorage<MachineInventoryComponent>& getMachineInventories() { return m_MachineInventories; }
    ComponentStorage<CraftingMachineComponent>& getCraftingMachines() { return m_CraftingMachines; }
    const RecipeDatabase& getRecipeDatabase() const { return m_RecipeDatabase; }
    Entity getPlayer() const { return m_Player; }
    void placeConveyorBelt(int tileX, int tileY, Direction direction);
    void removeConveyorBelt(int tileX, int tileY);
    void clearConveyorBelts();
    std::vector<std::tuple<int, int, Direction>> getConveyorBeltData() const;

private:
    Camera2D m_Camera;
    Entity m_Player;
    Entity m_Machine;

    Renderer* m_Renderer = nullptr;
    TileMap m_TileMap;
    SpriteAtlas m_TileMapAtlas;
    SpriteAtlas m_ConveyorAtlas;
    ChunkManager m_ChunkManager;

    ItemDatabase m_ItemDatabase;
    SpriteAtlas m_ItemAtlas;
    MachineDatabase m_MachineDatabase;

    EntityManager m_EntityManager;
    RecipeDatabase m_RecipeDatabase;

    ComponentStorage<PositionComponent> m_Positions;
    ComponentStorage<SpriteComponent> m_Sprites;
    ComponentStorage<AnimatedSpriteComponent> m_AnimatedSprites;
    ComponentStorage<VelocityComponent> m_Velocities;
    ComponentStorage<InputComponent> m_Inputs;
    ComponentStorage<CharacterStateComponent> m_CharacterStates;
    ComponentStorage<AnimationControllerComponent> m_AnimationControllers;
    ComponentStorage<CollisionComponent> m_Collisions;
    ComponentStorage<ConveyorBeltComponent> m_ConveyorBelts;
    ComponentStorage<ConveyorItemComponent> m_ConveyorItems;
    ComponentStorage<InventoryComponent> m_Inventories;
    ComponentStorage<MachineInventoryComponent> m_MachineInventories;
    ComponentStorage<CraftingMachineComponent> m_CraftingMachines;
    ComponentStorage<InteractionComponent> m_Interactions;

    AnimationLibrary m_AnimationLibrary;
    ConveyorManager m_ConveyorManager;

    AnimatedSprite m_AnimSpr;
    AnimatedSpriteComponent m_Comp;
    RenderSystem m_RenderSystem;
    AnimationSystem m_AnimationSystem;
    AnimatedRenderSystem m_AnimatedRenderSystem;
    MovementSystem m_MovementSystem;
    InputSystem m_InputSystem;
    MovementInputSystem m_MovementInputSystem;
    CharacterStateSystem m_CharacterStateSystem;
    AnimationStateSystem m_AnimationStateSystem;
    CollisionSystem m_CollisionSystem;
    ConveyorSystem m_ConveyorSystem;
    CraftingSystem m_CraftingSystem;
    InteractionSystem m_InteractionSystem;

};

#endif //PROJECTCOMPLEXITY_WORLD_H
