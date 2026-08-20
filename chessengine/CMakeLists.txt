add_library(chessengine STATIC
    src/attackeddetector.cpp
    src/chessboard.cpp
    src/chessengine.cpp
    src/moveanalyzer.cpp
    src/movegenerator.cpp
    src/position_applier.cpp
)

target_include_directories(chessengine
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)