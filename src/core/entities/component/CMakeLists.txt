target_sources(
        ProjectComplexity PRIVATE
        PositionComponent.hpp
        SpriteComponent.hpp
        AnimatedSpriteComponent.hpp
        VelocityComponent.hpp
        InputComponent.hpp
        CharacterStateComponent.hpp
        AnimationControllerComponent.hpp
        CollisionComponent.hpp
        InventoryComponent.hpp
        MachineComponent.hpp
        MachineInventoryComponent.hpp
        CraftingMachineComponent.hpp
        MinerComponent.hpp
        ConveyorBeltComponent.hpp
        ConveyorItemComponent.hpp
        InteractionComponent.hpp
        FluidPipeComponent.hpp
        FluidTankComponent.hpp
        FluidPortComponent.hpp
        FluidPumpComponent.hpp
        MachineFluidComponent.hpp
        MachineFluidPortLinkComponent.hpp

)

add_subdirectory(robot)