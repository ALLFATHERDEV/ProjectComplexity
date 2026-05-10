#include "WorldScreen.hpp"

#include "imgui.h"
#include "../Game.hpp"

namespace {
    const char* goapActionTypeToString(GoapActionType action) {
        switch (action) {
            case GoapActionType::MOVE_TO_PICKUP: return "MoveToPickup";
            case GoapActionType::PICKUP_ITEM: return "PickUpItem";
            case GoapActionType::MOVE_TO_DROPOFF: return "MoveToDropoff";
            case GoapActionType::DROP_OFF_ITEM: return "DropOffItem";
            default: return "Unknown";
        }
    }

    const char* robotTaskStatusToString(RobotTaskStatus status) {
        switch (status) {
            case RobotTaskStatus::PENDING: return "Pending";
            case RobotTaskStatus::RESERVED: return "Reserved";
            case RobotTaskStatus::COMPLETED: return "Completed";
            case RobotTaskStatus::INVALID: return "Invalid";
            case RobotTaskStatus::BLOCKED: return "Blocked";
            default: return "Unknown";
        }
    }
}

WorldScreen::WorldScreen(ScreenContext &context) : m_Context(context) {
    m_World.setRenderer(&context.renderer);
    m_World.initializeWorld();

    int viewportWidth = 0;
    int viewportHeight = 0;
    SDL_GetRenderOutputSize(m_Context.renderer.getSDLRenderer(), &viewportWidth, &viewportHeight);

    m_TileMapEditor.clearTilePalettes();
    for (const auto& palette : m_World.getTilePalettes()) {
        m_TileMapEditor.addTilePalette(palette.name, palette.atlas);
    }

    m_MachineGUI.create(m_GUISystem, &m_GUIDragContext);
    m_MachineGUI.setViewportSize(static_cast<float>(viewportWidth), static_cast<float>(viewportHeight));
    m_MachineGUI.bind(&m_World.getMachineInventories(),
                      &m_World.getMachineFluids(),
                      &m_World.getCraftingMachines(),
                      &m_World.getMiners(),
                      &m_World.getRecipeDatabase(),
                      &m_World.getItemDatabase(),
                      &m_World.getInventories(),
                      m_World.getPlayer());

    m_GUIDragPreview = m_GUISystem.addElement<GUIDragPreview>();
    m_GUIDragPreview->setDragContext(&m_GUIDragContext);
}

void WorldScreen::onEnter() {
    IScreen::onEnter();
}

void WorldScreen::onExit() {
    IScreen::onExit();
}

void WorldScreen::update(float deltaTime) {
    int viewportWidth = 0;
    int viewportHeight = 0;
    SDL_GetRenderOutputSize(m_Context.renderer.getSDLRenderer(), &viewportWidth, &viewportHeight);
    m_MachineGUI.setViewportSize(static_cast<float>(viewportWidth), static_cast<float>(viewportHeight));

    m_DebugFpsAccumulator += deltaTime;
    ++m_DebugFpsFrameCount;
    if (m_DebugFpsAccumulator >= 0.25f) {
        m_DebugFps = static_cast<float>(m_DebugFpsFrameCount) / m_DebugFpsAccumulator;
        m_DebugFpsAccumulator = 0.0f;
        m_DebugFpsFrameCount = 0;
    }

    m_World.update(deltaTime);
    m_MachineGUI.update();
}

void WorldScreen::render() {
    m_World.render();

    Vec2f worldPos = getMouseWorldPosition();
    if (const Entity hovered = m_World.getHoveredMachine(worldPos.x, worldPos.y); hovered > 0) {
        m_World.renderMachineHighlight(hovered);
    }

    renderDraggedPlaceablePreview();
    m_Context.renderer.flushSpriteQueue();
    m_GUISystem.render(&m_Context.renderer);

    m_TileMapEditor.renderImGui(m_World);
    m_ItemDebugEditor.renderImGui(m_World);
    renderDebugOverlay();
}

void WorldScreen::handleEvent(SDL_Event &event) {
     if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F1) {
            m_TileMapEditor.setEnabled(!m_TileMapEditor.isEnabled());
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F2) {
            m_ItemDebugEditor.setEnabled(!m_ItemDebugEditor.isEnabled());
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F3 && !event.key.repeat) {
            m_ShowDebugOverlay = !m_ShowDebugOverlay;
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F4) {
            const Vec2f mouseWorldPosition = getMouseWorldPosition();
            const int mouseTileX = static_cast<int>(mouseWorldPosition.x) / 32;
            const int mouseTileY = static_cast<int>(mouseWorldPosition.y) / 32;
            m_World.addDebugFluidToTank(mouseTileX, mouseTileY, 250.0f);
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            m_World.getCamera().addZoom(event.wheel.y * 0.1f);
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_R && isDraggingConveyorPlaceableItem()) {
            rotateSelectedPlaceableDirection();
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
            m_GUIDragContext.suppressWorldPlacementUntilMouseRelease = false;
        }

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse && !isDraggingPlaceableItem()) {
            m_TileMapEditor.update(event, m_World, m_World.getCamera());
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB && !event.key.repeat) {
            m_MachineGUI.togglePlayerInventory();
        }
        m_GUISystem.handleEvent(event);

        if (!io.WantCaptureMouse) {
            tryPlaceDraggedItem(event);
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            if (m_MachineGUI.isOpen() || m_MachineGUI.isPlayerInventoryOpen()) {
                m_MachineGUI.close();
            }
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_E) {
            auto interactedEntity = m_World.tryInteract(event);
            if (interactedEntity.has_value()) {
                if (m_World.getMachineInventories().get(interactedEntity.value())) {
                    m_MachineGUI.open(interactedEntity.value());
                } else if (m_World.getInventories().get(interactedEntity.value()) &&
                           interactedEntity.value() != m_World.getPlayer()) {
                    m_MachineGUI.openStorage(interactedEntity.value());
                }
            }
        }

        if (!io.WantCaptureKeyboard) {
            m_World.handleInput(event);
        }
}

bool WorldScreen::isDraggingPlaceableItem() const {
    if (!m_GUIDragContext.isDragging || m_GUIDragContext.draggedStack.isEmpty() || !m_GUIDragContext.draggedStack.item) {
        return false;
    }

    return m_GUIDragContext.draggedStack.item->isPlaceable;
}

bool WorldScreen::isDraggingConveyorPlaceableItem() const {
    if (!isDraggingPlaceableItem()) {
        return false;
    }

    return m_GUIDragContext.draggedStack.item->placesConveyorBelt;
}

bool WorldScreen::tryPlaceDraggedItem(const SDL_Event &event) {
    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT) {
        return false;
    }

    if (!isDraggingPlaceableItem()) {
        return false;
    }

    if (m_GUIDragContext.suppressWorldPlacementUntilMouseRelease) {
        return false;
    }

    const ItemDefinition* item = m_GUIDragContext.draggedStack.item;
    if (!item) {
        return false;
    }

    float mouseX = 0.0f;
    float mouseY = 0.0f;
    SDL_GetMouseState(&mouseX, &mouseY);

    Vec2f worldPosition = getMouseWorldPosition();
    const int tileX = static_cast<int>(worldPosition.x) / 32;
    const int tileY = static_cast<int>(worldPosition.y) / 32;

    if (!m_World.placeItem(*item, tileX, tileY, m_SelectedPlaceableDirection)) {
        return false;
    }

    m_GUIDragContext.draggedStack.amount--;
    if (m_GUIDragContext.draggedStack.amount <= 0) {
        m_GUIDragContext.clear();
    }

    return true;
}

void WorldScreen::renderDraggedPlaceablePreview() {
    if (!isDraggingPlaceableItem()) {
        return;
    }

    const ItemDefinition* item = m_GUIDragContext.draggedStack.item;
    if (!item) {
        return;
    }

    float mouseX = 0.0f;
    float mouseY = 0.0f;
    SDL_GetMouseState(&mouseX, &mouseY);

    Vec2f worldPosition = getMouseWorldPosition();
    const int tileX = static_cast<int>(worldPosition.x) / 32;
    const int tileY = static_cast<int>(worldPosition.y) / 32;

    m_World.renderPlacementPreview(*item, tileX, tileY, m_SelectedPlaceableDirection);
}

void WorldScreen::renderDebugOverlay() {
    if (!m_ShowDebugOverlay) {
        return;
    }

    const Vec2f playerPosition = m_World.getPlayerPosition();
    const int tileX = static_cast<int>(playerPosition.x) / 32;
    const int tileY = static_cast<int>(playerPosition.y) / 32;
    const ChunkManager& chunkManager = m_World.getChunkManager();
    const int chunkX = chunkManager.getCenterChunkX();
    const int chunkY = chunkManager.getCenterChunkY();
    const float zoom = m_World.getCamera().getZoom();
    const std::vector<FluidNetwork>& fluidNetworks = m_World.getFluidSystem().getNetworks();
    const auto& robotTasks = m_World.getRobotTaskBoard().getTasks();
    const auto& robotEntities = m_World.getRobots().getEntities();
    const size_t queuedSprites = m_Context.renderer.getLastQueuedSpriteCount();
    const size_t textureSwitches = m_Context.renderer.getLastTextureSwitchCount();

    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowBgAlpha(0.75f);
    ImGui::SetNextWindowPos(
        ImVec2(static_cast<float>(Game::WINDOW_WIDTH) - 16.0f, static_cast<float>(Game::WINDOW_HEIGHT) - 16.0f),
        ImGuiCond_Always,
        ImVec2(1.0f, 1.0f)
    );

    Vec2f mouseWorldPosition = getMouseWorldPosition();

    if (ImGui::Begin("DebugOverlay", nullptr, windowFlags)) {
        ImGui::Text("FPS: %.1f", m_DebugFps);
        ImGui::Text("Grid: %d, %d", tileX, tileY);
        ImGui::Text("Chunk: %d, %d", chunkX, chunkY);
        ImGui::Text("Zoom: %.2f", zoom);
        ImGui::Text("Hovered Entity: %d", m_World.getHoveredMachine(mouseWorldPosition.x, mouseWorldPosition.y));
        ImGui::Text("Queued Sprites: %d", static_cast<int>(queuedSprites));
        ImGui::Text("Texture Switches: %d", static_cast<int>(textureSwitches));
        ImGui::Text("Fluid Networks: %d", static_cast<int>(fluidNetworks.size()));
        for (const FluidNetwork& network : fluidNetworks) {
            ImGui::Text("Net %d P:%d T:%d Cap:%.0f Amt:%.0f %s",
                        network.id,
                        static_cast<int>(network.pipes.size()),
                        static_cast<int>(network.tanks.size()),
                        network.totalCapacity,
                        network.fluid.amount,
                        network.fluid.fluid ? network.fluid.fluid->displayName.c_str() : "Empty");
        }

        ImGui::SeparatorText("Robot Tasks");
        ImGui::Text("Tasks: %d", static_cast<int>(robotTasks.size()));
        for (const RobotTask& task : robotTasks) {
            ImGui::Text("Task %d %s item=%s amt=%d pickup=%d dropoff=%d by=%d",
                        task.id,
                        robotTaskStatusToString(task.status),
                        task.itemName.c_str(),
                        task.amount,
                        task.pickupEntity,
                        task.dropOffEntity,
                        task.reservedBy);
            if (!task.lastFailureReason.empty()) {
                ImGui::Text("  reason: %s", task.lastFailureReason.c_str());
            }
        }

        ImGui::SeparatorText("Robots");
        for (Entity robot : robotEntities) {
            const RobotBrainComponent* brain = m_World.getRobotBrains().get(robot);
            const RobotCarryComponent* carry = m_World.getRobotCarries().get(robot);
            const PositionComponent* pos = m_World.getComponents().m_Positions.get(robot);

            if (!brain || !carry || !pos) {
                continue;
            }

            ImGui::Text("Robot %d pos=(%.0f, %.0f) task=%d plan=%d idx=%d target=%d",
                        robot,
                        pos->position.x,
                        pos->position.y,
                        brain->currentTaskId,
                        static_cast<int>(brain->currentPlan.size()),
                        static_cast<int>(brain->currentPlanIndex),
                        brain->runtimeTargetEntity);

            if (!carry->carriedItem.isEmpty() && carry->carriedItem.item) {
                ImGui::Text("  carry: %s x%d",
                            carry->carriedItem.item->uniqueName.c_str(),
                            carry->carriedItem.amount);
            } else {
                ImGui::Text("  carry: empty");
            }

            if (brain->currentPlanIndex < brain->currentPlan.size()) {
                ImGui::Text("  action: %s", goapActionTypeToString(brain->currentPlan[brain->currentPlanIndex]));
            } else {
                ImGui::Text("  action: none");
            }
        }

        ImGui::Text("F3: Toggle debug overlay");
        ImGui::Text("F4: Fill hovered tank with Water");
    }
    ImGui::End();
}

Vec2f WorldScreen::getMouseWorldPosition() {
    float mouseX;
    float mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    Camera2D& camera = m_World.getCamera();
    return Vec2f(mouseX / camera.getZoom() + camera.getX(), mouseY / camera.getZoom() + camera.getY());
}

void WorldScreen::rotateSelectedPlaceableDirection() {
    switch (m_SelectedPlaceableDirection) {
        case Direction::RIGHT:
            m_SelectedPlaceableDirection = Direction::DOWN;
            break;
        case Direction::DOWN:
            m_SelectedPlaceableDirection = Direction::LEFT;
            break;
        case Direction::LEFT:
            m_SelectedPlaceableDirection = Direction::UP;
            break;
        case Direction::UP:
        default:
            m_SelectedPlaceableDirection = Direction::RIGHT;
            break;
    }
}
