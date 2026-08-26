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
    src/components/image.cpp
    src/components/menu.cpp
    src/components/menu_item.cpp
    src/components/radio_button.cpp
    src/components/slider.cpp
    src/components/tab_control.cpp
    src/components/tab_item.cpp
    src/components/text_input.cpp
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
    src/core/text_edit_state.cpp
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

    set(UI_FRAMEWORK_SDL_DLLS
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/bin/SDL3.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image/bin/SDL3_image.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf/bin/SDL3_ttf.dll"
    )

    foreach(SDL_DLL IN LISTS UI_FRAMEWORK_SDL_DLLS)
        if(NOT EXISTS "${SDL_DLL}")
            message(FATAL_ERROR "Required SDL runtime DLL not found: ${SDL_DLL}")
        endif()
    endforeach()

    add_executable(ui_framework_input_tests tests/core/input_system_tests.cpp)
    add_executable(ui_framework_input_mutation_tests tests/core/input_system_mutation_tests.cpp)
    add_executable(ui_framework_node_tree_tests tests/core/node_tree_tests.cpp)
    add_executable(ui_framework_node_tree_regression_tests tests/core/node_tree_regression_tests.cpp)
    add_executable(ui_framework_ui_manager_regression_tests tests/core/ui_manager_regression_tests.cpp)
    add_executable(ui_framework_modal_system_regression_tests tests/core/modal_system_regression_tests.cpp)
    add_executable(ui_framework_scroll_system_regression_tests tests/core/scroll_system_regression_tests.cpp)
    add_executable(ui_framework_scroll_system_behavior_tests tests/core/scroll_system_behavior_tests.cpp)
    add_executable(ui_framework_layout_system_regression_tests tests/core/layout_system_regression_tests.cpp)
    add_executable(ui_framework_text_edit_state_tests tests/core/text_edit_state_tests.cpp)
    add_executable(ui_framework_text_input_tests tests/core/text_input_tests.cpp)
    add_executable(ui_framework_image_tests tests/core/image_tests.cpp)

    target_include_directories(ui_framework_input_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_input_mutation_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_node_tree_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_node_tree_regression_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_ui_manager_regression_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_modal_system_regression_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_scroll_system_regression_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_scroll_system_behavior_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_layout_system_regression_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_text_edit_state_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_text_input_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
    target_include_directories(ui_framework_image_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/core)

    target_link_libraries(ui_framework_input_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_input_mutation_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_node_tree_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_node_tree_regression_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_ui_manager_regression_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_modal_system_regression_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_scroll_system_regression_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_scroll_system_behavior_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_layout_system_regression_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_text_edit_state_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_text_input_tests PRIVATE ui_framework)
    target_link_libraries(ui_framework_image_tests PRIVATE ui_framework)

    add_test(NAME ui_framework_input_tests COMMAND ui_framework_input_tests)
    add_test(NAME ui_framework_input_mutation_tests COMMAND ui_framework_input_mutation_tests)
    add_test(NAME ui_framework_node_tree_tests COMMAND ui_framework_node_tree_tests)
    add_test(NAME ui_framework_node_tree_regression_tests COMMAND ui_framework_node_tree_regression_tests)
    add_test(NAME ui_framework_ui_manager_regression_tests COMMAND ui_framework_ui_manager_regression_tests)
    add_test(NAME ui_framework_modal_system_regression_tests COMMAND ui_framework_modal_system_regression_tests)
    add_test(NAME ui_framework_scroll_system_regression_tests COMMAND ui_framework_scroll_system_regression_tests)
    add_test(NAME ui_framework_scroll_system_behavior_tests COMMAND ui_framework_scroll_system_behavior_tests)
    add_test(NAME ui_framework_layout_system_regression_tests COMMAND ui_framework_layout_system_regression_tests)
    add_test(NAME ui_framework_text_edit_state_tests COMMAND ui_framework_text_edit_state_tests)
    add_test(NAME ui_framework_text_input_tests COMMAND ui_framework_text_input_tests)
    add_test(NAME ui_framework_image_tests COMMAND ui_framework_image_tests)

    foreach(TEST_TARGET IN ITEMS ui_framework_input_tests ui_framework_input_mutation_tests ui_framework_node_tree_tests ui_framework_node_tree_regression_tests ui_framework_ui_manager_regression_tests ui_framework_modal_system_regression_tests ui_framework_scroll_system_regression_tests ui_framework_scroll_system_behavior_tests ui_framework_layout_system_regression_tests ui_framework_text_edit_state_tests ui_framework_text_input_tests ui_framework_image_tests)
        foreach(SDL_DLL IN LISTS UI_FRAMEWORK_SDL_DLLS)
            add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL_DLL}"
                    "$<TARGET_FILE_DIR:${TEST_TARGET}>"
                VERBATIM
            )
        endforeach()
    endforeach()
endif()