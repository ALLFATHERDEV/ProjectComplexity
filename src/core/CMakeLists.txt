target_sources(ProjectComplexity PRIVATE
        Game.hpp
        Game.cpp
        Logger.hpp
        Logger.cpp
)

add_subdirectory(camera)
add_subdirectory(editor)
add_subdirectory(entities)
add_subdirectory(graphics)
add_subdirectory(map)
add_subdirectory(util)
add_subdirectory(world)
add_subdirectory(inventory)
add_subdirectory(gui)
add_subdirectory(machine)
add_subdirectory(recipe)
add_subdirectory(fluid)
