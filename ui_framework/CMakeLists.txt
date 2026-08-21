cmake_minimum_required(VERSION 3.20)

project(ui_framework
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================
# External dependencies
# ==========================

list(APPEND CMAKE_PREFIX_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf
)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)

# ==========================
# UI Framework library
# ==========================

add_library(ui_framework STATIC
    src/core/ui_manager.cpp
    src/core/nodetree.cpp
    src/core/node.cpp
    src/core/panelnode.cpp
    src/core/stackpanelnode.cpp
    src/core/layoutmanager.cpp
    src/core/modalmanager.cpp
    src/core/inputmanager.cpp
    src/core/primitives.cpp
    src/core/linear_layout.cpp
    src/core/text_primitive.cpp
    src/detail/scroll_system.cpp
)

# ==========================
# Include directories
# ==========================

target_include_directories(ui_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(ui_framework PUBLIC cxx_std_20)

# ==========================
# Link dependencies
# ==========================

target_link_libraries(ui_framework
    PUBLIC
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
)
