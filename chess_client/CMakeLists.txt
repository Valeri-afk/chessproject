cmake_minimum_required(VERSION 3.20)

add_executable(chess_client
    src/main.cpp
)

target_include_directories(chess_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(chess_client
    PRIVATE
        chessengine
        ui_framework
)

add_custom_command(TARGET chess_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:chess_client>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        "$<TARGET_FILE_DIR:chess_client>/fonts"
)

set(CHESSCLIENT_SDL_DLLS
    "${CMAKE_CURRENT_SOURCE_DIR}/../ui_framework/external/SDL3/bin/SDL3.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/../ui_framework/external/SDL3_image/bin/SDL3_image.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/../ui_framework/external/SDL3_ttf/bin/SDL3_ttf.dll"
)

foreach(SDL_DLL IN LISTS CHESSCLIENT_SDL_DLLS)
    if(NOT EXISTS "${SDL_DLL}")
        message(FATAL_ERROR "Required SDL runtime DLL not found: ${SDL_DLL}")
    endif()

    add_custom_command(TARGET chess_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL_DLL}"
            "$<TARGET_FILE_DIR:chess_client>"
        VERBATIM
    )
endforeach()