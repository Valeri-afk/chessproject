#pragma once

#include <optional>
#include <vector>

#include <chessengine/chessengine.hpp>
#include <chessengine/types.hpp>

class ChessController
{
private:
    ChessEngine &chessEngine;

    std::optional<MoveOutcome> lastMoveOutcome_;
    std::optional<Position> selectedPosition_;
    std::vector<Move> legalMoves_;
    std::vector<Move> promotionMoves_;

    void preparePromotionMoves(const Position &target);
    void clearSelectionState();

public:
    explicit ChessController(ChessEngine &chessEngine);

    void selectSquare(Position position);
    bool selectPromotion(PromotionType promotion);
    void resetSelection();

    bool hasSelection() const noexcept;
    const std::optional<Position> &selectedPosition() const noexcept;
    const std::vector<Move> &legalMoves() const noexcept;
    const std::vector<Move> &promotionMoves() const noexcept;
    const std::optional<MoveOutcome> &lastMoveOutcome() const noexcept;
};
