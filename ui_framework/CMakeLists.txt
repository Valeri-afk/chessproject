cmake_minimum_required(VERSION 3.20)

project(ui_framework VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf
)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)

add_library(ui_framework STATIC
    src/components/button.cpp
    src/components/checkbox.cpp
    src/components/dropdown.cpp
    src/components/menu.cpp
    src/components/menu_item.cpp
    src/components/radio_button.cpp
    src/components/slider.cpp
    src/components/tab_control.cpp
    src/components/tab_item.cpp
    src/components/toggle_button.cpp
    src/components/typography.cpp
    src/core/ui_manager.cpp
    src/core/node_tree.cpp
    src/core/node.cpp
    src/core/panel_node.cpp
    src/core/stack_panel_node.cpp
    src/core/text_layout.cpp
    src/core/text_content.cpp
    src/core/text_renderer.cpp
    src/core/layout_system.cpp
    src/core/modal_system.cpp
    src/core/modal_system_backdrop.cpp
    src/core/input_system.cpp
    src/core/primitives.cpp
    src/core/layout_constraints.cpp
    src/core/linear_layout.cpp
    src/core/scroll_system.cpp
)

target_include_directories(ui_framework
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(ui_framework PUBLIC cxx_std_20)

target_link_libraries(ui_framework
    PUBLIC
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
)

option(UI_FRAMEWORK_BUILD_TESTS "Build ui_framework tests" OFF)

if(UI_FRAMEWORK_BUILD_TESTS)
    enable_testing()

    add_executable(ui_framework_input_tests
        tests/core/input_system_tests.cpp
        tests/core/input_system_mutation_tests.cpp
    )

    target_include_directories(ui_framework_input_tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(ui_framework_input_tests
        PRIVATE ui_framework
    )

    add_test(NAME ui_framework_input_tests COMMAND ui_framework_input_tests)
endif()
