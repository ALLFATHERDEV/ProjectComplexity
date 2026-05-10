#include "RobotSystem.hpp"

#include "../../Logger.hpp"
#include "../component/InventoryComponent.hpp"
#include "../component/PositionComponent.hpp"
#include "../component/robot/RobotCarryComponent.hpp"
#include "../component/robot/RobotComponent.hpp"

void RobotSystem::update(float deltaTime, RobotSystemContext &context, RobotTaskBoard &taskBoard) {
   const auto& robotEntities = context.robots.getEntities();

    for (Entity robot : robotEntities) {
        RobotComponent* robotComponent = context.robots.get(robot);
        RobotBrainComponent* brain = context.robotBrains.get(robot);
        RobotCarryComponent* carry = context.robotCarries.get(robot);

        if (!robotComponent || !brain || !carry) {
            continue;
        }

        if (brain->currentTaskId == -1) {
            RobotTask* task = taskBoard.findFirstUnreservedDeliverTask();
            if (task) {
                taskBoard.reserveTask(task->id, robot);
                brain->currentTaskId = task->id;
                brain->planDirty = true;
                brain->currentGoal = "deliver_item";

                LOG_INFO("RobotSystem: robot {} reserved task {}", robot, task->id);
            }
        }

        if (brain->currentTaskId == -1) {
            brain->currentPlan.clear();
            brain->currentPlanIndex = 0;
            continue;
        }

        RobotTask* task = taskBoard.getTaskById(brain->currentTaskId);
        if (!task ||
            task->status == RobotTaskStatus::COMPLETED ||
            task->status == RobotTaskStatus::INVALID ||
            task->status == RobotTaskStatus::BLOCKED) {
            brain->currentTaskId = -1;
            brain->currentGoal.clear();
            brain->planDirty = true;
            brain->currentPlan.clear();
            brain->currentPlanIndex = 0;
            continue;
        }

        if (brain->planDirty) {
            GoapState startState = buildWorldState(robot,
                                                   *robotComponent,
                                                   *brain,
                                                   *carry,
                                                   *task,
                                                   context.positions,
                                                   context.inventories);

            GoapState goalState = buildGoalState(*task);

            std::vector<GoapActionType> plan;
            const bool success = m_Planner.buildPlan(startState, goalState, plan);

            if (success) {
                brain->currentPlan = std::move(plan);
                brain->currentPlanIndex = 0;
                brain->planDirty = false;

                LOG_INFO("RobotSystem: robot {} planned {} actions for task {}",
                         robot,
                         static_cast<int>(brain->currentPlan.size()),
                         task->id);
            } else {
                brain->currentPlan.clear();
                brain->currentPlanIndex = 0;
                brain->planDirty = false;
                taskBoard.blockTask(task->id, "no_plan_found");

                LOG_WARN("RobotSystem: robot {} failed to build plan for task {}",
                         robot,
                         task->id);
                continue;
            }
        }

        if (brain->currentPlan.empty()) {
            continue;
        }

        if (brain->currentPlanIndex >= brain->currentPlan.size()) {
            continue;
        }

        const RobotActionStatus actionStatus = executeCurrentAction(robot,
                                                                   *robotComponent,
                                                                   *brain,
                                                                   *carry,
                                                                   *task,
                                                                   context.positions,
                                                                   context.inventories,
                                                                   taskBoard,
                                                                   deltaTime);

        if (actionStatus == RobotActionStatus::SUCCESS) {
            brain->currentPlanIndex++;

            if (brain->currentPlanIndex >= brain->currentPlan.size()) {
                LOG_INFO("RobotSystem: robot {} finished plan for task {}", robot, task->id);
                if (task->status == RobotTaskStatus::RESERVED) {
                    brain->planDirty = true;
                    brain->currentPlan.clear();
                    brain->currentPlanIndex = 0;
                }
            }
        } else if (actionStatus == RobotActionStatus::FAILED) {
            brain->planDirty = true;
            brain->currentPlan.clear();
            brain->currentPlanIndex = 0;

            LOG_WARN("RobotSystem: robot {} action failed, replanning task {}", robot, task->id);
        } else if (actionStatus == RobotActionStatus::BLOCKED) {
            taskBoard.blockTask(task->id, "action_blocked");
            brain->currentTaskId = -1;
            brain->currentGoal.clear();
            brain->currentPlan.clear();
            brain->currentPlanIndex = 0;
            brain->planDirty = true;
            brain->runtimeTargetEntity = 0;
            brain->runtimeMoveTarget = {};

            LOG_WARN("RobotSystem: robot {} blocked task {}", robot, task->id);
        } else if (actionStatus == RobotActionStatus::INVALID) {
            taskBoard.invalidateTask(task->id, "action_invalid");
            brain->currentTaskId = -1;
            brain->currentGoal.clear();
            brain->currentPlan.clear();
            brain->currentPlanIndex = 0;
            brain->planDirty = true;
            brain->runtimeTargetEntity = 0;
            brain->runtimeMoveTarget = {};

            LOG_WARN("RobotSystem: robot {} invalidated task {}", robot, task->id);
        }

        if (task->status == RobotTaskStatus::COMPLETED) {
            brain->currentTaskId = -1;
            brain->currentGoal.clear();
            brain->currentPlan.clear();
            brain->currentPlanIndex = 0;
            brain->planDirty = true;
            brain->runtimeTargetEntity = 0;
            brain->runtimeMoveTarget = {};
        }
    }
}

GoapState RobotSystem::buildWorldState(Entity robot, const RobotComponent& robotComponent, const RobotBrainComponent &brain, const RobotCarryComponent &carry, const RobotTask &task, ComponentStorage<PositionComponent> &positions, ComponentStorage<InventoryComponent> &inventories) const {
    GoapState state;
    state.set(GoapFact::HAS_TASK, brain.currentTaskId != -1);
    state.set(GoapFact::CARRYING_ITEM, !carry.carriedItem.isEmpty());

    const InventoryComponent* pickupInventory = inventories.get(task.pickupEntity);
    const InventoryComponent* dropOffInventory = inventories.get(task.dropOffEntity);
    if (pickupInventory)
        state.set(GoapFact::PICKUP_HAS_ITEM, inventoryHasItem(pickupInventory->inventory, task.itemName, task.amount));

    if (dropOffInventory)
        state.set(GoapFact::DROP_OFF_HAS_SPACE, inventoryHasSpaceForItem(dropOffInventory->inventory, task.itemName));

    const PositionComponent* robotPosition = positions.get(robot);
    const PositionComponent* pickupPosition = positions.get(task.pickupEntity);
    const PositionComponent* dropOffPosition = positions.get(task.dropOffEntity);

    if (robotPosition && pickupPosition)
        state.set(GoapFact::AT_PICKUP, isInRange(robotPosition->position, pickupPosition->position, robotComponent.interactionRange));

    if (robotPosition && dropOffPosition)
        state.set(GoapFact::AT_DROP_OFF, isInRange(robotPosition->position, dropOffPosition->position, robotComponent.interactionRange));

    state.set(GoapFact::ITEM_DELIVERED, false);

    return state;

}

GoapState RobotSystem::buildGoalState(const RobotTask &task) const {
    GoapState goal;

    switch (task.type) {
        case RobotTaskType::DELIVER_ITEM:
            goal.set(GoapFact::ITEM_DELIVERED, true);
            break;
    }
    return goal;
}

bool RobotSystem::isInRange(const Vec2f &a, const Vec2f &b, float range) const {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return dx * dx + dy * dy <= range * range;
}

bool RobotSystem::inventoryHasItem(const InventoryGrid &inventory, const std::string &itemName, int requiredAmount) const {
    int foundAmount = 0;

    for (const InventorySlot& slot : inventory.getSlots()) {
        if (slot.isEmpty() || !slot.stack.item)
            continue;
        if (slot.stack.item->uniqueName != itemName)
            continue;
        foundAmount += slot.stack.amount;

        if (foundAmount >= requiredAmount)
            return true;
    }

    return false;
}

bool RobotSystem::inventoryHasSpaceForItem(const InventoryGrid &inventory, const std::string &itemName) const {
    for (const InventorySlot& slot : inventory.getSlots()) {
        if (slot.isEmpty()) {
            return true;
        }

        if (!slot.stack.item) {
            continue;
        }

        if (slot.stack.item->uniqueName == itemName &&
            slot.stack.amount < slot.stack.item->maxStackSize) {
            return true;
            }
    }

    return false;
}

RobotActionStatus RobotSystem::executeCurrentAction(Entity robot, RobotComponent &robotComponent, RobotBrainComponent &brain, RobotCarryComponent &carry, RobotTask &task, ComponentStorage<PositionComponent> &positions, ComponentStorage<InventoryComponent> &inventories, RobotTaskBoard &taskBoard, float deltaTime) const {
    if (brain.currentPlanIndex >= brain.currentPlan.size()) {
        return RobotActionStatus::SUCCESS;
    }

    switch (brain.currentPlan[brain.currentPlanIndex]) {
        case GoapActionType::MOVE_TO_PICKUP:
            return executeMoveToPickup(robot, robotComponent, brain, task, positions, deltaTime);
        case GoapActionType::PICKUP_ITEM:
            return executePickUpItem(robot, robotComponent, carry, brain, task, positions, inventories);
        case GoapActionType::MOVE_TO_DROPOFF:
            return executeMoveToDropOff(robot, robotComponent, brain, task, positions, deltaTime);
        case GoapActionType::DROP_OFF_ITEM:
            return executeDropOffItem(robot, carry, brain, task, positions, inventories, taskBoard);
    }

    return RobotActionStatus::FAILED;
}

RobotActionStatus RobotSystem::executeMoveToPickup(Entity robot, RobotComponent &robotComponent, RobotBrainComponent &brain, const RobotTask &task, ComponentStorage<PositionComponent> &positions, float deltaTime) const {
    PositionComponent* robotPosition = positions.get(robot);
    const PositionComponent* pickupPosition = positions.get(task.pickupEntity);

    if (!robotPosition || !pickupPosition) {
        return RobotActionStatus::INVALID;
    }

    brain.runtimeTargetEntity = task.pickupEntity;
    brain.runtimeMoveTarget = pickupPosition->position;

    const bool arrived = moveTowards(robotPosition->position, pickupPosition->position, robotComponent.moveSpeed, deltaTime, robotComponent.interactionRange);
    return arrived ? RobotActionStatus::SUCCESS : RobotActionStatus::RUNNING;
}

RobotActionStatus RobotSystem::executePickUpItem(Entity robot, RobotComponent& robotComponent, RobotCarryComponent &carry, RobotBrainComponent &brain, RobotTask &task, ComponentStorage<PositionComponent> &positions, ComponentStorage<InventoryComponent> &inventories) const {
    PositionComponent* robotPosition = positions.get(robot);
    const PositionComponent* pickupPosition = positions.get(task.pickupEntity);

    if (!robotPosition || !pickupPosition) {
        return RobotActionStatus::INVALID;
    }

    if (!isInRange(robotPosition->position, pickupPosition->position, 24.0f)) {
        return RobotActionStatus::FAILED;
    }

    if (!carry.carriedItem.isEmpty()) {
        return RobotActionStatus::SUCCESS;
    }

    InventoryComponent* pickupInventory = inventories.get(task.pickupEntity);
    if (!pickupInventory) {
        return RobotActionStatus::INVALID;
    }

    for (InventorySlot& slot : pickupInventory->inventory.getSlots()) {
        if (slot.isEmpty() || !slot.stack.item) {
            continue;
        }

        if (slot.stack.item->uniqueName != task.itemName) {
            continue;
        }

        const int movedAmount = std::min(task.amount, std::min(slot.stack.amount, robotComponent.carryCapacity));
        carry.carriedItem.item = slot.stack.item;
        carry.carriedItem.amount = movedAmount;

        slot.stack.amount -= movedAmount;
        if (slot.stack.amount <= 0) {
            slot.stack.clear();
        }

        brain.runtimeTargetEntity = task.pickupEntity;
        LOG_INFO("RobotSystem: robot {} picked up {}x {}", robot, movedAmount, task.itemName);
        return RobotActionStatus::SUCCESS;
    }

    return RobotActionStatus::BLOCKED;
}

RobotActionStatus RobotSystem::executeMoveToDropOff(Entity robot, RobotComponent &robotComponent, RobotBrainComponent &brain, const RobotTask &task, ComponentStorage<PositionComponent> &positions, float deltaTime) const {
    PositionComponent* robotPosition = positions.get(robot);
    const PositionComponent* dropOffPosition = positions.get(task.dropOffEntity);

    if (!robotPosition || !dropOffPosition) {
        return RobotActionStatus::INVALID;
    }

    brain.runtimeTargetEntity = task.dropOffEntity;
    brain.runtimeMoveTarget = dropOffPosition->position;

    const bool arrived = moveTowards(robotPosition->position,
                                     dropOffPosition->position,
                                     robotComponent.moveSpeed,
                                     deltaTime,
                                     robotComponent.interactionRange);

    return arrived ? RobotActionStatus::SUCCESS : RobotActionStatus::RUNNING;
}

RobotActionStatus RobotSystem::executeDropOffItem(Entity robot, RobotCarryComponent &carry, RobotBrainComponent &brain, RobotTask &task, ComponentStorage<PositionComponent> &positions, ComponentStorage<InventoryComponent> &inventories, RobotTaskBoard &taskBoard) const {
    const PositionComponent* robotPosition = positions.get(robot);
    const PositionComponent* dropOffPosition = positions.get(task.dropOffEntity);

    if (!robotPosition || !dropOffPosition) {
        return RobotActionStatus::INVALID;
    }

    brain.runtimeTargetEntity = task.dropOffEntity;
    brain.runtimeMoveTarget = dropOffPosition->position;

    if (!isInRange(robotPosition->position, dropOffPosition->position, 24.0f)) {
        return RobotActionStatus::FAILED;
    }

    if (carry.carriedItem.isEmpty() || !carry.carriedItem.item) {
        return RobotActionStatus::FAILED;
    }

    InventoryComponent* dropOffInventory = inventories.get(task.dropOffEntity);
    if (!dropOffInventory) {
        return RobotActionStatus::INVALID;
    }

    if (!dropOffInventory->inventory.addItem(carry.carriedItem.item, carry.carriedItem.amount)) {
        return RobotActionStatus::BLOCKED;
    }

    LOG_INFO("RobotSystem: robot {} dropped off {}x {}",
             robot,
             carry.carriedItem.amount,
             carry.carriedItem.item->uniqueName);

    task.amount -= carry.carriedItem.amount;
    carry.carriedItem.clear();

    if (task.amount <= 0) {
        taskBoard.completeTask(task.id);
    }

    return RobotActionStatus::SUCCESS;
}

bool RobotSystem::moveTowards(Vec2f& currentPosition, const Vec2f& targetPosition, float speed, float deltaTime, float stopRange) const {
    const float dx = targetPosition.x - currentPosition.x;
    const float dy = targetPosition.y - currentPosition.y;
    const float distanceSq = dx * dx + dy * dy;

    if (distanceSq <= stopRange * stopRange) {
        return true;
    }

    const float distance = std::sqrt(distanceSq);
    if (distance <= 0.0001f)
        return true;

    const float maxStep = speed * deltaTime;
    if (maxStep >= distance) {
        currentPosition = targetPosition;
        return true;
    }

    currentPosition.x += (dx / distance) * maxStep;
    currentPosition.y += (dy / distance) * maxStep;
    return false;
}
