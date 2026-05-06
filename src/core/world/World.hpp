#ifndef PROJECTCOMPLEXITY_WORLD_H
#define PROJECTCOMPLEXITY_WORLD_H
#include <string>
#include <vector>

#include "AnimationLibrary.hpp"
#include "ChunkManager.hpp"
#include "ConveyorManager.hpp"
#include "FluidManager.hpp"
#include "../entities/ComponentStorage.hpp"
#include "../entities/EntityManager.hpp"
#include "../entities/component/AnimatedSpriteComponent.hpp"
#include "../entities/component/CollisionComponent.hpp"
#include "../entities/component/ConveyorBeltComponent.hpp"
#include "../entities/component/ConveyorItemComponent.hpp"
#include "../entities/component/CraftingMachineComponent.hpp"
#include "../entities/component/FluidPipeComponent.hpp"
#include "../entities/component/FluidPumpComponent.hpp"
#include "../entities/component/FluidPortComponent.hpp"
#include "../entities/component/FluidTankComponent.hpp"
#include "../entities/component/InteractionComponent.hpp"
#include "../entities/component/InventoryComponent.hpp"
#include "../entities/component/MachineComponent.hpp"
#include "../entities/component/MachineFluidComponent.hpp"
#include "../entities/component/MachineFluidPortLinkComponent.hpp"
#include "../entities/component/MachineInventoryComponent.hpp"
#include "../entities/component/MinerComponent.hpp"
#include "../entities/component/PositionComponent.hpp"
#include "../entities/component/SpriteComponent.hpp"
#include "../entities/systems/AnimatedRenderSystem.hpp"
#include "../entities/systems/AnimationStateSystem.hpp"
#include "../entities/systems/AnimationSystem.hpp"
#include "../entities/systems/CharacterStateSystem.hpp"
#include "../entities/systems/CollisionSystem.hpp"
#include "../entities/systems/ConveyorSystem.hpp"
#include "../entities/systems/CraftingSystem.hpp"
#include "../entities/systems/FluidSystem.hpp"
#include "../entities/systems/InputSystem.hpp"
#include "../entities/systems/InteractionSystem.hpp"
#include "../entities/systems/MachineFluidIOSystem.hpp"
#include "../entities/systems/MovementInputSystem.hpp"
#include "../entities/systems/MovementSystem.hpp"
#include "../entities/systems/MiningSystem.hpp"
#include "../entities/systems/RenderSystem.hpp"
#include "../graphics/Renderer.hpp"
#include "../graphics/SpriteAtlas.hpp"
#include "../fluid/FluidDatabase.hpp"
#include "../inventory/ItemDatabase.hpp"
#include "../machine/MachineDatabase.hpp"
#include "../map/TileMetadataDatabase.hpp"
#include "../map/TileMap.hpp"

struct InputComponent;
struct VelocityComponent;

class World {
public:
    struct TilePaletteInfo {
        std::string name;
        SpriteAtlas* atlas = nullptr;
    };

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
    SpriteAtlas& getOreVeinAtlas() { return m_OreVeinsAtlas; }
    const std::vector<TilePaletteInfo>& getTilePalettes() const { return m_TilePalettes; }
    SpriteAtlas* getTilePalette(const std::string& name) const;
    SpriteAtlas& getItemAtlas() { return m_ItemAtlas; }
    ItemDatabase& getItemDatabase() { return m_ItemDatabase; }
    const ItemDatabase& getItemDatabase() const { return m_ItemDatabase; }
    ChunkManager& getChunkManager() { return m_ChunkManager; }
    const ChunkManager& getChunkManager() const { return m_ChunkManager; }
    ComponentStorage<InventoryComponent>& getInventories() { return m_Inventories; }
    ComponentStorage<MachineInventoryComponent>& getMachineInventories() { return m_MachineInventories; }
    ComponentStorage<CraftingMachineComponent>& getCraftingMachines() { return m_CraftingMachines; }
    ComponentStorage<MinerComponent>& getMiners() { return m_Miners; }
    ComponentStorage<FluidPipeComponent>& getFluidPipes() { return m_FluidPipes; }
    ComponentStorage<FluidTankComponent>& getFluidTanks() { return m_FluidTanks; }
    ComponentStorage<FluidPumpComponent>& getFluidPumps() { return m_FluidPumps; }
    ComponentStorage<FluidPortComponent>& getFluidPorts() { return m_FluidPorts; }
    ComponentStorage<MachineFluidComponent>& getMachineFluids() { return m_MachineFluids; }
    FluidSystem& getFluidSystem() { return m_FluidSystem; }
    const FluidSystem& getFluidSystem() const { return m_FluidSystem; }
    const RecipeDatabase& getRecipeDatabase() const { return m_RecipeDatabase; }
    const FluidDatabase& getFluidDatabase() const { return m_FluidDatabase; }
    Entity getPlayer() const { return m_Player; }
    Vec2f getPlayerPosition() const;
    bool canPlaceItem(const ItemDefinition& item, int tileX, int tileY) const;
    bool placeItem(const ItemDefinition& item, int tileX, int tileY, Direction direction = Direction::RIGHT);
    void renderPlacementPreview(const ItemDefinition& item, int tileX, int tileY, Direction direction = Direction::RIGHT) const;
    bool placeMachine(const std::string& machineUniqueName, int tileX, int tileY, Direction direction = Direction::RIGHT);
    void clearMachines();
    std::vector<std::tuple<std::string, int, int, Direction>> getMachinePlacementData() const;
    void placeConveyorBelt(int tileX, int tileY, Direction direction);
    void removeConveyorBelt(int tileX, int tileY);
    void placeFluidPipe(int tileX, int tileY, Direction direction = Direction::RIGHT);
    void removeFluidPipe(int tileX, int tileY);
    void placeFluidTank(int tileX, int tileY);
    void removeFluidTank(int tileX, int tileY);
    void placeFluidPump(int tileX, int tileY, Direction direction);
    void removeFluidPump(int tileX, int tileY);
    bool addDebugFluidToTank(int tileX, int tileY, float amount);
    void clearConveyorBelts();
    std::vector<std::tuple<int, int, Direction>> getConveyorBeltData() const;
    Entity getHoveredMachine(float worldX, float worldY) const;
    void renderMachineHighlight(Entity machine) const;

private:
    bool isAreaBlockedByEntity(const SDL_FRect& rect) const;
    bool satisfiesMachinePlacementCondition(const MachineDefinition& machineDefinition, int tileX, int tileY) const;
    Sprite getMachineSprite(const MachineDefinition& machineDefinition) const;
    void registerTilePalette(const std::string& name, SpriteAtlas* atlas);

    Camera2D m_Camera;
    Entity m_Player;
    Entity m_Machine;

    Renderer* m_Renderer = nullptr;
    TileMap m_TileMap;
    SpriteAtlas m_TileMapAtlas;
    SpriteAtlas m_ConveyorAtlas;
    SpriteAtlas m_FluidAtlas;
    SpriteAtlas m_OreVeinsAtlas;
    std::vector<TilePaletteInfo> m_TilePalettes;
    ChunkManager m_ChunkManager;

    ItemDatabase m_ItemDatabase;
    SpriteAtlas m_ItemAtlas;
    MachineDatabase m_MachineDatabase;
    TileMetadataDatabase m_TileMetadataDatabase;

    EntityManager m_EntityManager;
    RecipeDatabase m_RecipeDatabase;
    FluidDatabase m_FluidDatabase;

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
    ComponentStorage<FluidPipeComponent> m_FluidPipes;
    ComponentStorage<FluidTankComponent> m_FluidTanks;
    ComponentStorage<FluidPumpComponent> m_FluidPumps;
    ComponentStorage<FluidPortComponent> m_FluidPorts;
    ComponentStorage<MachineFluidComponent> m_MachineFluids;
    ComponentStorage<MachineFluidPortLinkComponent> m_MachineFluidPortLinks;
    ComponentStorage<InventoryComponent> m_Inventories;
    ComponentStorage<MachineComponent> m_MachineEntities;
    ComponentStorage<MachineInventoryComponent> m_MachineInventories;
    ComponentStorage<CraftingMachineComponent> m_CraftingMachines;
    ComponentStorage<MinerComponent> m_Miners;
    ComponentStorage<InteractionComponent> m_Interactions;

    AnimationLibrary m_AnimationLibrary;
    ConveyorManager m_ConveyorManager;

    AnimatedSprite m_AnimSpr;
    AnimatedSpriteComponent m_Comp;
    RenderSystem m_RenderSystem;
    AnimationSystem m_AnimationSystem;
    MovementSystem m_MovementSystem;
    InputSystem m_InputSystem;
    MovementInputSystem m_MovementInputSystem;
    CharacterStateSystem m_CharacterStateSystem;
    AnimationStateSystem m_AnimationStateSystem;
    CollisionSystem m_CollisionSystem;
    ConveyorSystem m_ConveyorSystem;
    CraftingSystem m_CraftingSystem;
    FluidSystem m_FluidSystem;
    MachineFluidIOSystem m_MachineFluidIOSystem;
    FluidManager m_FluidManager;
    MiningSystem m_MiningSystem;
    InteractionSystem m_InteractionSystem;

};

#endif //PROJECTCOMPLEXITY_WORLD_H
