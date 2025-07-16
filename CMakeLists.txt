cmake_minimum_required(VERSION 3.14)

project(OpenRWEditor VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Qt
find_package(Qt6 COMPONENTS Widgets OpenGL REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add source files
set(SOURCE_FILES
    src/main.cpp
    src/editor_window.cpp
    src/gta_loader.cpp
    src/asset_manager.cpp
    src/entity_system.cpp
    src/scene_manager.cpp
    src/file_formats/dff_parser.cpp
    src/file_formats/txd_parser.cpp
    src/file_formats/ide_parser.cpp
    src/file_formats/ipl_parser.cpp
    src/file_formats/dat_parser.cpp
    src/viewport/viewport_widget.cpp
    src/viewport/camera_controller.cpp
    src/ui/property_inspector.cpp
    src/ui/asset_browser.cpp
    src/ui/world_outliner.cpp
)

# Add header files
set(HEADER_FILES
    src/editor_window.h
    src/gta_loader.h
    src/asset_manager.h
    src/entity_system.h
    src/scene_manager.h
    src/file_formats/dff_parser.h
    src/file_formats/txd_parser.h
    src/file_formats/ide_parser.h
    src/file_formats/ipl_parser.h
    src/file_formats/dat_parser.h
    src/viewport/viewport_widget.h
    src/viewport/camera_controller.h
    src/ui/property_inspector.h
    src/ui/asset_browser.h
    src/ui/world_outliner.h
    src/common/types.h
    src/common/math_utils.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets 
    Qt6::OpenGL
    ${OPENGL_LIBRARIES}
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file_formats
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewport
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Copy assets directory to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

