cmake_minimum_required(VERSION 3.20)
project(chess_ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаём исполняемый файл
add_executable(chess_ui)

# Добавляем исходники
target_sources(chess_ui
PRIVATE
    src/main.cpp
    src/core/input_manager.cpp
    src/core/resource_manager.cpp
    src/core/sdl_wrapper.cpp
    src/core/texture.cpp
    src/ui/primitives/primitives.cpp
    src/ui/base/ui_manager.cpp
    src/ui/base/container.cpp
    src/ui/base/widget.cpp
    src/ui/base/textlayout.cpp
    src/ui/components/button.cpp
    src/ui/components/menu.cpp
    src/ui/components/panel.cpp
    src/ui/components/label.cpp
    src/ui/components/modal.cpp
)

# Подключаем заголовочные файлы
target_include_directories(chess_ui
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf/include
)

# Пути к библиотекам
target_link_directories(chess_ui
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf/lib
)

# Линкуем библиотеки
target_link_libraries(chess_ui
PRIVATE
    SDL3
    SDL3_image
    SDL3_ttf
)

# Копируем DLL после сборки
add_custom_command(TARGET chess_ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/bin/SDL3.dll"
        $<TARGET_FILE_DIR:chess_ui>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image/bin/SDL3_image.dll"
        $<TARGET_FILE_DIR:chess_ui>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf/bin/SDL3_ttf.dll"
        $<TARGET_FILE_DIR:chess_ui>
)

add_custom_command(TARGET chess_ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        "$<TARGET_FILE_DIR:chess_ui>/fonts"
)

add_custom_command(TARGET chess_ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:chess_ui>/assets"
)