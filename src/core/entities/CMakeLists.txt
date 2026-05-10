target_sources(ProjectComplexity PRIVATE
        Entity.hpp
        EntityManager.hpp
        ComponentStorage.hpp
        ComponentRegistry.hpp
)

add_subdirectory(component)
add_subdirectory(systems)
