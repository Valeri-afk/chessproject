cmake_minimum_required(VERSION 3.20)

add_executable(chess_client
    src/main.cpp
    src/core/input_manager.cpp
    src/core/resource_manager.cpp
    src/core/sdl_wrapper.cpp
    src/core/texture.cpp
    src/game/game.cpp
    src/game/ui/components/chessboard_component.cpp
    src/game/ui/components/promotion_component.cpp
)

add_custom_command(TARGET chess_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/bin/SDL3.dll"
        $<TARGET_FILE_DIR:chess_client>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image/bin/SDL3_image.dll"
        $<TARGET_FILE_DIR:chess_client>
)

add_custom_command(TARGET chess_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:chess_client>/assets"
)

target_include_directories(chess_client
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image/include
)

target_link_directories(chess_client
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image/lib
)

target_link_libraries(chess_client
PRIVATE
    chessengine
    SDL3
    SDL3_image
)