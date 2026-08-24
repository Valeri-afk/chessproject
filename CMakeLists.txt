cmake_minimum_required(VERSION 3.20)
project(chess_ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ui_framework)

add_executable(chess_ui
    chess_client/src/main.cpp
)

target_link_libraries(chess_ui
PRIVATE
    ui_framework
)
