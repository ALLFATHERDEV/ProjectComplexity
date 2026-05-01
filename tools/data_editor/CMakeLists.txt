set(DATA_EDITOR_SOURCE_FILES
        main.cpp
)

add_executable(ProjectComplexityEditor ${DATA_EDITOR_SOURCE_FILES})

target_link_libraries(ProjectComplexityEditor PUBLIC SDL3::SDL3)
target_link_libraries(ProjectComplexityEditor PUBLIC imgui)
target_link_libraries(ProjectComplexityEditor PUBLIC nlohmann_json::nlohmann_json)

add_custom_command(TARGET ProjectComplexityEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:ProjectComplexityEditor>
        $<TARGET_FILE_DIR:ProjectComplexityEditor>
        COMMAND_EXPAND_LISTS
)
