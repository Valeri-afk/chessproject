#pragma once
#include <vector>
#include <chessengine/types.hpp>
#include <chessengine/chessengine.hpp>
#include "core/input_manager.hpp"

class ChessController
{
private:
    InputManager &input;
    ChessEngine &chessEngine;

    std::optional<MoveOutcome> lastMoveOutcome;
    std::optional<Position> activePiece;
    std::vector<Move> activeMoves;

    std::vector<Move> promotionMoves;

public:
    ChessController(InputManager &input, ChessEngine &chessEngine);

    void selectPiece(Position pos);
    void resetSelect();

    void handlePieceSelect(Position pos);
    void handlePromotion(float x, float y);
    void handleClick();
};
