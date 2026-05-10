#pragma once
#include <string>
#include <vector>

#include "RobotTask.hpp"
#include "../entities/Entity.hpp"

class RobotTaskBoard {
public:
    int addDeliverItemTask(const std::string& itemName, int amount, Entity pickupEntity, Entity dropOffEntity);

    RobotTask* getTaskById(int taskId);
    const RobotTask* getTaskById(int taskId) const;

    RobotTask* findFirstUnreservedDeliverTask();
    void reserveTask(int taskId, Entity robot);
    void releaseTask(int taskId);
    void completeTask(int taskId);
    void invalidateTask(int taskId, const std::string& reason);
    void blockTask(int taskId, const std::string& reason);

    const std::vector<RobotTask>& getTasks() const { return m_Tasks; }

private:
    int m_NextTaskId = 1;
    std::vector<RobotTask> m_Tasks;
};
