#include "RobotTaskBoard.hpp"

int RobotTaskBoard::addDeliverItemTask(const std::string &itemName, int amount, Entity pickupEntity, Entity dropOffEntity) {
    RobotTask task;
    task.id = m_NextTaskId++;
    task.type = RobotTaskType::DELIVER_ITEM;
    task.itemName = itemName;
    task.amount = amount;
    task.pickupEntity = pickupEntity;
    task.dropOffEntity = dropOffEntity;
    m_Tasks.push_back(task);
    return task.id;
}

RobotTask* RobotTaskBoard::getTaskById(int taskId) {
    for (RobotTask& task : m_Tasks) {
        if (task.id == taskId) {
            return &task;
        }
    }
    return nullptr;
}

const RobotTask* RobotTaskBoard::getTaskById(int taskId) const {
    for (const RobotTask& task : m_Tasks) {
        if (task.id == taskId) {
            return &task;
        }
    }
    return nullptr;
}

RobotTask* RobotTaskBoard::findFirstUnreservedDeliverTask() {
    for (RobotTask& task : m_Tasks) {
        if (task.type != RobotTaskType::DELIVER_ITEM) {
            continue;
        }

        if (task.status != RobotTaskStatus::PENDING) {
            continue;
        }

        return &task;
    }

    return nullptr;
}

void RobotTaskBoard::reserveTask(int taskId, Entity robot) {
    RobotTask* task = getTaskById(taskId);
    if (!task) {
        return;
    }

    task->status = RobotTaskStatus::RESERVED;
    task->reservedBy = robot;
    task->lastFailureReason.clear();
}

void RobotTaskBoard::releaseTask(int taskId) {
    RobotTask* task = getTaskById(taskId);
    if (!task) {
        return;
    }

    if (task->status == RobotTaskStatus::RESERVED) {
        task->status = RobotTaskStatus::PENDING;
    }
    task->reservedBy = 0;
}

void RobotTaskBoard::completeTask(int taskId) {
    RobotTask* task = getTaskById(taskId);
    if (!task) {
        return;
    }

    task->status = RobotTaskStatus::COMPLETED;
    task->reservedBy = 0;
    task->lastFailureReason.clear();
}

void RobotTaskBoard::invalidateTask(int taskId, const std::string& reason) {
    RobotTask* task = getTaskById(taskId);
    if (!task) {
        return;
    }

    task->status = RobotTaskStatus::INVALID;
    task->reservedBy = 0;
    task->lastFailureReason = reason;
}

void RobotTaskBoard::blockTask(int taskId, const std::string& reason) {
    RobotTask* task = getTaskById(taskId);
    if (!task) {
        return;
    }

    task->status = RobotTaskStatus::BLOCKED;
    task->reservedBy = 0;
    task->lastFailureReason = reason;
}
