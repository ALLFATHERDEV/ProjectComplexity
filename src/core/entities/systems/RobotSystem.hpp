#pragma once
#include "../ComponentStorage.hpp"
#include "../../inventory/InventoryGrid.hpp"
#include "../../robot/RobotTaskBoard.hpp"
#include "../../robot/goap/GoapPlanner.hpp"
#include "../../robot/goap/GoapState.hpp"

struct InventoryComponent;
struct PositionComponent;
struct RobotCarryComponent;
struct RobotBrainComponent;
struct RobotComponent;

struct RobotSystemContext {
    ComponentStorage<RobotComponent>& robots;
    ComponentStorage<RobotBrainComponent>& robotBrains;
    ComponentStorage<RobotCarryComponent>& robotCarries;
    ComponentStorage<PositionComponent>& positions;
    ComponentStorage<InventoryComponent>& inventories;
};

enum class RobotActionStatus {
    RUNNING,
    SUCCESS,
    FAILED,
    BLOCKED,
    INVALID
};

class RobotSystem {
public:
    void update(float deltaTime, RobotSystemContext& context, RobotTaskBoard& taskBoard);

private:
    GoapState buildWorldState(Entity robot, const RobotComponent& robotComponent, const RobotBrainComponent& brain, const RobotCarryComponent& carry, const RobotTask& task, ComponentStorage<PositionComponent>& positions, ComponentStorage<InventoryComponent>& inventories) const;
    GoapState buildGoalState(const RobotTask& task) const;
    bool isInRange(const Vec2f& a, const Vec2f& b, float range) const;
    bool inventoryHasItem(const InventoryGrid& inventory, const std::string& itemName, int requiredAmount) const;
    bool inventoryHasSpaceForItem(const InventoryGrid& inventory, const std::string& itemName) const;
    RobotActionStatus executeCurrentAction(Entity robot, RobotComponent& robotComponent, RobotBrainComponent& brain, RobotCarryComponent& carry, RobotTask& task, ComponentStorage<PositionComponent>& positions, ComponentStorage<InventoryComponent>& inventories, RobotTaskBoard& taskBoard, float deltaTime) const;
    RobotActionStatus executeMoveToPickup(Entity robot, RobotComponent& robotComponent, RobotBrainComponent& brain, const RobotTask& task, ComponentStorage<PositionComponent>& positions, float deltaTime) const;
    RobotActionStatus executePickUpItem(Entity robot, RobotComponent& robotComponent, RobotCarryComponent& carry, RobotBrainComponent& brain, RobotTask& task, ComponentStorage<PositionComponent>& positions, ComponentStorage<InventoryComponent>& inventories) const;
    RobotActionStatus executeMoveToDropOff(Entity robot, RobotComponent& robotComponent, RobotBrainComponent& brain, const RobotTask& task, ComponentStorage<PositionComponent>& positions, float deltaTime) const;
    RobotActionStatus executeDropOffItem(Entity robot, RobotCarryComponent& carry, RobotBrainComponent& brain, RobotTask& task, ComponentStorage<PositionComponent>& positions, ComponentStorage<InventoryComponent>& inventories, RobotTaskBoard& taskBoard) const;
    bool moveTowards(Vec2f& currentPosition, const Vec2f& targetPosition, float speed, float deltaTime, float stopRange) const;

    GoapPlanner m_Planner;
};
