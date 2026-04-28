cmake_minimum_required(VERSION 4.0)
# Project Name
project(ProjectComplexity)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SOURCE_FILES src/main.cpp
        src/main.cpp
)

#SDL 3
add_subdirectory(libs/SDL EXCLUDE_FROM_ALL)
set(SDLIMAGE_VENDORED OFF)
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPG OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_PNG_LIBPNG OFF)    # SDL3 includes built-in support for loading PNGs. If you need more than what that supports, enable this option.
add_subdirectory(libs/SDL_image EXCLUDE_FROM_ALL)

#SDL 3 TTF
set(SDLTTF_VENDORED ON)
add_subdirectory(libs/SDL_ttf EXCLUDE_FROM_ALL)

#ImGUI
add_library(imgui STATIC
        libs/ImGui/imgui.cpp
        libs/ImGui/imgui_draw.cpp
        libs/ImGui/imgui_tables.cpp
        libs/ImGui/imgui_widgets.cpp
        libs/ImGui/imgui_demo.cpp

        libs/ImGui/backends/imgui_impl_sdl3.cpp
        libs/ImGui/backends/imgui_impl_sdlrenderer3.cpp
)

#FMT
include(FetchContent)
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281)
FetchContent_MakeAvailable(fmt)

#NLohmann JSON Library
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)


# ProjectComplexity executable
add_executable(ProjectComplexity ${SOURCE_FILES})
add_subdirectory(src/core)

target_include_directories(imgui PUBLIC libs/ImGui libs/ImGui/backends)

target_link_libraries(imgui PUBLIC SDL3::SDL3)
target_link_libraries(ProjectComplexity PUBLIC SDL3::SDL3)
target_link_libraries(ProjectComplexity PUBLIC SDL3_image::SDL3_image)
target_link_libraries(ProjectComplexity PUBLIC SDL3_ttf::SDL3_ttf)
target_link_libraries(ProjectComplexity PUBLIC fmt::fmt)
target_link_libraries(ProjectComplexity PUBLIC imgui)
target_link_libraries(ProjectComplexity PRIVATE nlohmann_json::nlohmann_json)